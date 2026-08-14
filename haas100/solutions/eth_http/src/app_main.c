/*
 * eth_http: HaaS100 Ethernet + embedded web server demo.
 *
 * Stock AliOS Things pattern (see solutions/ota_demo/otaappdemo.c and
 * components/netmgr/example/netmgr_example.c):
 *   - startup.c brings up lwIP + the CH395Q netif at boot (eth_lwip_tcpip_init)
 *   - the app only starts the event/netmgr services and waits for the
 *     EVENT_NETMGR_DHCP_SUCCESS event, then serves the web app on :80.
 */
#include <stdio.h>
#include <string.h>
#include <aos/kernel.h>
#include "uservice/uservice.h"
#include "uservice/eventid.h"
#include "netmgr.h"
#include "netmgr_ethernet.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "http_server.h"

static volatile int g_dhcp_ok = 0;
static uint32_t g_ip = 0;
static uint32_t g_mask = 0;
static uint32_t g_gw = 0;

static void netmgr_dhcp_event_cb(uint32_t event_id, const void *param, void *context)
{
    if (event_id == EVENT_NETMGR_DHCP_SUCCESS || event_id == EVENT_ETHERNET_DHCP_SUCCESS) {
        printf("eth_http: DHCP success event\r\n");
        g_dhcp_ok = 1;
    }
}

int application_start(int argc, char *argv[])
{
    int retry = 0;

    printf("eth_http: netmgr init\r\n");
    event_service_init(NULL);
    netmgr_service_init(NULL);
    netmgr_set_auto_reconnect(NULL, true);

    event_subscribe(EVENT_NETMGR_DHCP_SUCCESS, netmgr_dhcp_event_cb, NULL);
    event_subscribe(EVENT_ETHERNET_DHCP_SUCCESS, netmgr_dhcp_event_cb, NULL);

    printf("eth_http: waiting for link/DHCP\r\n");
    while (retry++ < 300) {
        if (g_dhcp_ok) {
            break;
        }
        if ((retry % 50) == 0) {
            printf("eth_http: waiting for DHCP...\r\n");
        }
        aos_msleep(100);
    }

    /* Static IP override. The DHCP-assigned IP (192.168.5.40) collides with
     * the host bridge; use a unique static IP in the same subnet instead. */
    {
        netmgr_hdl_t eth_hdl = netmgr_get_dev("/dev/eth0");
        netmgr_ifconfig_info_t cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.dhcp_en = false;
        strcpy(cfg.ip_addr, "192.168.5.50");
        strcpy(cfg.mask, "255.255.255.0");
        strcpy(cfg.gw, "192.168.5.1");
        strcpy(cfg.dns_server, "192.168.5.1");
        netmgr_set_ifconfig(eth_hdl, &cfg);
        printf("eth_http: using static IP 192.168.5.50\r\n");
    }

    /* The DHCP events fired, so netmgr obtained an IP on the CH395 lwIP netif.
     * netmgr_eth_get_ip_stat() reads it from the netmgr connection info. */
    {
        extern int netmgr_eth_get_ip_stat(char *ip, char *mask, char *gw, char *dns, bool dhcp_en);
        char ip[16] = {0}, mask[16] = {0}, gw[16] = {0}, dns[16] = {0};
        int i;
        for (i = 0; i < 20; i++) {
            if (netmgr_eth_get_ip_stat(ip, mask, gw, dns, false) == 0 && ip[0] != '0') {
                break;
            }
            aos_msleep(100);
        }
        if (ip[0] != '\0' && strcmp(ip, "0.0.0.0") != 0) {
            printf("eth_http: LAN IP %s\r\n", ip);
            printf("eth_http: listening on :80 - open http://%s/\r\n", ip);
        } else {
            printf("eth_http: no DHCP lease\r\n");
        }
    }

    printf("eth_http: starting http server\r\n");
    if (http_server_start() != 0) {
        printf("eth_http: failed to start http server\r\n");
    }

    while (1) {
        aos_msleep(60000);
    }

    return 0;
}
