/*
 * wifi_http: HaaS100 Wi-Fi + embedded web server demo.
 * Connects to a Wi-Fi AP (edit WIFI_SSID/WIFI_PWD below), gets an IP via
 * DHCP, and serves the e_server single-page app on :80 using lwIP sockets.
 *
 * NOTE: netmgr_wifi_connect() first runs a Wi-Fi scan (ioctl START_SCAN /
 * START_SPECIFIED_SCAN), which hangs inside the prebuilt bwifi_config_scan()
 * in libmcuhaas1000.a (no source available). We therefore call the driver
 * CONNECT ioctl directly (bwifi_connect_to_ssid() needs no scan) and let the
 * netmgr event pipeline (EVENT_WIFI_CONNECTED -> netif up -> DHCP ->
 * EVENT_NETMGR_DHCP_SUCCESS) complete the rest.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <aos/kernel.h>
#include "netmgr.h"
#include "netmgr_wifi.h"
#include "uservice/uservice.h"
#include "uservice/eventid.h"
#include "vfsdev/wifi_dev.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "http_server.h"
#include "wifi_config.h"

#ifndef DEFAULT_AP
#error "wifi_config.h is missing DEFAULT_AP. Copy wifi_config.h.example to wifi_config.h and fill in your credentials."
#endif

static struct netif *find_netif_with_ip(void)
{
    struct netif *n;

    for (n = netif_list; n != NULL; n = n->next) {
        if (!ip4_addr_isany(netif_ip4_addr(n))) {
            return n;
        }
    }
    return NULL;
}

int application_start(int argc, char *argv[])
{
    netmgr_hdl_t hdl = NULL;
    struct netif *n = NULL;
    int retry = 0;

    printf("wifi_http: starting lwIP\r\n");
    tcpip_init(NULL, NULL);

    printf("wifi_http: netmgr init\r\n");
    event_service_init(NULL);
    netmgr_service_init(NULL);
    netmgr_set_auto_reconnect(NULL, true);
    netmgr_wifi_set_auto_save_ap(true);

    hdl = netmgr_get_dev("/dev/wifi0");
    if (!hdl) {
        printf("wifi_http: wifi dev /dev/wifi0 not found\r\n");
    } else {
        /* Bypass netmgr_wifi_connect()'s mandatory scan (hangs in the
         * prebuilt bwifi_config_scan). Call the driver CONNECT directly;
         * netmgr still drives netif-up + DHCP via the WIFI events. */
        wifi_config_t type;
        memset(&type, 0, sizeof(type));
        type.mode = WIFI_MODE_STA;
        snprintf(type.ssid, sizeof(type.ssid), "%s", DEFAULT_AP);
        snprintf(type.password, sizeof(type.password), "%s", DEFAULT_PASSWD);
        printf("wifi_http: connecting to '%s' (direct ioctl, no scan)\r\n", type.ssid);
        if (ioctl(hdl, WIFI_DEV_CMD_CONNECT, &type) != 0) {
            printf("wifi_http: direct connect ioctl failed\r\n");
        }
    }

    /* wait for a DHCP lease (up to 30 s) */
    while (retry++ < 300) {
        n = find_netif_with_ip();
        if (n) {
            break;
        }
        if ((retry % 50) == 0) {
            printf("wifi_http: waiting for IP...\r\n");
        }
        aos_msleep(100);
    }

    n = find_netif_with_ip();
    if (n) {
        char ip[16];
        ip4addr_ntoa_r(netif_ip4_addr(n), ip, sizeof(ip));
        printf("wifi_http: LAN IP %s\r\n", ip);
        printf("wifi_http: open http://%s/\r\n", ip);
    } else {
        printf("wifi_http: no DHCP lease - check SSID/password\r\n");
    }

    printf("wifi_http: starting http server\r\n");
    if (http_server_start() != 0) {
        printf("wifi_http: failed to start http server\r\n");
    }

    while (1) {
        aos_msleep(60000);
    }

    return 0;
}
