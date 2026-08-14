/*
 * Embedded HTTP server for the HaaS100 board, mirroring e_server's API.
 * Serves the gzipped single-page app (web_assets.h) and three endpoints:
 *   GET  /             gzip page (Content-Encoding: gzip)
 *   GET  /api/leds     {"leds":[0,0,0,0,0]}   (5 on-board LEDs)
 *   POST /api/leds     body {"leds":[0,1,0,1,0]} -> applied
 *   GET  /api/adc      {"ch0_mv":..,"bat_mv":..,"ch2_mv":..,"ts":..}
 *   GET  /api/info     {"arch","lan_ip","public_ip":null,...}
 *   GET  /public/*     raw embedded images
 *
 * Uses the lwIP BSD-socket API (start the TCP/IP stack with tcpip_init()
 * before calling http_server_start()).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "aos/kernel.h"
#include "lwip/sockets.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "led.h"
#include "hal_gpadc.h"
#include "http_server.h"
#include "web_assets.h"

#define HTTP_PORT 80

static int leds[5] = { 0, 0, 0, 0, 0 };

static void send_all(int fd, const void *buf, size_t len)
{
    const char *p = buf;
    while (len > 0) {
        int n = lwip_send(fd, p, len, 0);
        if (n <= 0) return;
        p += n;
        len -= (size_t)n;
    }
}

static void send_response(int fd, int code, const char *status,
                          const char *ctype, const char *encoding,
                          const void *body, size_t len)
{
    char hdr[512];
    int n = snprintf(hdr, sizeof(hdr),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: %s\r\n"
                     "%s"                        /* optional Content-Encoding */
                     "Content-Length: %zu\r\n"
                     "Cache-Control: no-store\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     code, status, ctype,
                     encoding ? encoding : "",
                     len);
    send_all(fd, hdr, (size_t)n);
    if (len) send_all(fd, body, len);
}

static void api_leds(int fd, const char *method, const char *body)
{
    if (strcmp(method, "POST") == 0 && body)
    {
        const char *b = strchr(body, '[');
        if (b)
        {
            int a[5] = { 0, 0, 0, 0, 0 };
            int got = sscanf(b + 1, "%d , %d , %d , %d , %d",
                             &a[0], &a[1], &a[2], &a[3], &a[4]);
            if (got > 0)
                for (int i = 0; i < got && i < 5; i++) {
                    leds[i] = a[i] ? 1 : 0;
                    led_switch(i + 1, leds[i] ? LED_ON : LED_OFF);
                }
        }
    }

    char resp[64];
    int n = snprintf(resp, sizeof(resp), "{\"leds\":[%d,%d,%d,%d,%d]}",
                     leds[0], leds[1], leds[2], leds[3], leds[4]);
    send_response(fd, 200, "OK", "application/json", NULL, resp, (size_t)n);
}

/* Read one GPADC channel in mV (first 3 board channels: chan0, battery, chan2). */
static uint16_t adc_read_mv(enum HAL_GPADC_CHAN_T chan)
{
    HAL_GPADC_MV_T volt = HAL_GPADC_BAD_VALUE;

    hal_gpadc_open(chan, HAL_GPADC_ATP_ONESHOT, NULL);
    aos_msleep(2);
    if (hal_gpadc_get_volt(chan, &volt)) {
        hal_gpadc_close(chan);
        return volt;
    }
    hal_gpadc_close(chan);
    return 0;
}

static void api_adc(int fd)
{
    uint16_t ch0 = adc_read_mv(HAL_GPADC_CHAN_0);
    uint16_t bat = adc_read_mv(HAL_GPADC_CHAN_BATTERY);
    uint16_t ch2 = adc_read_mv(HAL_GPADC_CHAN_2);

    char resp[128];
    int n = snprintf(resp, sizeof(resp),
                     "{\"ch0_mv\":%u,\"bat_mv\":%u,\"ch2_mv\":%u,\"ts\":%u}",
                     ch0, bat, ch2, (unsigned)aos_now_ms());
    send_response(fd, 200, "OK", "application/json", NULL, resp, (size_t)n);
}

static void api_info(int fd)
{
    char lan[16] = "0.0.0.0";
    extern int netmgr_eth_get_ip_stat(char *ip, char *mask, char *gw, char *dns, bool dhcp_en);
    char ip[16] = {0}, mask[16] = {0}, gw[16] = {0}, dns[16] = {0};
    if (netmgr_eth_get_ip_stat(ip, mask, gw, dns, false) == 0 && ip[0] != '\0') {
        strncpy(lan, ip, sizeof(lan) - 1);
    }

    char resp[256];
    int len = snprintf(resp, sizeof(resp),
                       "{\"arch\":\"cortex-m33\",\"lan_ip\":\"%s\","
                       "\"public_ip\":null,\"geo\":null,\"weather\":null,\"ts\":%u}",
                       lan, (unsigned)aos_now_ms());
    send_response(fd, 200, "OK", "application/json", NULL, resp, (size_t)len);
}

static void serve_public(int fd, const char *path)
{
    for (unsigned i = 0; i < embedded_files_count; i++) {
        if (strcmp(path, embedded_files[i].path) == 0) {
            send_response(fd, 200, "OK", embedded_files[i].ctype, NULL,
                          embedded_files[i].data, embedded_files[i].len);
            return;
        }
    }
    send_response(fd, 404, "Not Found", "text/plain", NULL, "not found", 9);
}

static void route(int fd, const char *method, const char *path, const char *body)
{
    if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        send_response(fd, 200, "OK", "text/html; charset=utf-8",
                      "Content-Encoding: gzip\r\n",
                      index_html_gz, index_html_gz_len);
    } else if (strcmp(path, "/api/leds") == 0) {
        api_leds(fd, method, body);
    } else if (strcmp(path, "/api/adc") == 0) {
        api_adc(fd);
    } else if (strcmp(path, "/api/info") == 0) {
        api_info(fd);
    } else if (strcmp(path, "/favicon.ico") == 0) {
        send_response(fd, 204, "No Content", "text/plain", NULL, NULL, 0);
    } else if (strncmp(path, "/public/", 8) == 0) {
        serve_public(fd, path);
    } else {
        send_response(fd, 404, "Not Found", "text/plain", NULL, "not found", 9);
    }
}

/* Read one HTTP request (headers + body). Returns 1 on success. */
static int read_request(int fd, char *buf, size_t cap, char **body_out)
{
    size_t got = 0;
    char *body = NULL;
    *body_out = NULL;

    while (got < cap - 1) {
        int r = lwip_recv(fd, buf + got, (int)(cap - 1 - got), 0);
        if (r <= 0) break;
        got += (size_t)r;
        buf[got] = 0;

        char *hdr_end = strstr(buf, "\r\n\r\n");
        if (!hdr_end) continue;

        long cl = 0;
        const char *clh = strstr(buf, "Content-Length:");
        if (clh) {
            clh += 15;
            while (*clh == ' ') clh++;
            cl = atol(clh);
        }
        size_t want = (size_t)(hdr_end - buf) + 4 + (size_t)cl;
        if (want >= cap) return 0;
        while (got < want) {
            r = lwip_recv(fd, buf + got, (int)(want - got), 0);
            if (r <= 0) break;
            got += (size_t)r;
        }
        buf[got] = 0;
        if (got >= want && cl > 0) body = hdr_end + 4;
        *body_out = body;
        return got >= want;
    }
    return 0;
}

#define REQ_BUF_SIZE 16384

static void handle_client(int fd)
{
    /* The request buffer must live on the heap: the server task stack is only
     * 4 KB, and a 16 KB stack-local buffer would overflow it and corrupt
     * adjacent kernel objects (this caused a memory-corruption crash). */
    char *buf = malloc(REQ_BUF_SIZE);
    if (buf == NULL) {
        return;
    }
    char *body = NULL;

    if (read_request(fd, buf, REQ_BUF_SIZE, &body)) {
        char method[8] = { 0 }, path[1024] = { 0 }, ver[16] = { 0 };
        if (sscanf(buf, "%7s %1023s %15s", method, path, ver) == 3) {
            char *q = strchr(path, '?');
            if (q) *q = 0;
            route(fd, method, path, body);
        }
    }
    free(buf);
}

static void http_server_task(void *arg)
{
    (void)arg;
    int srv = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) {
        printf("eth_http: socket failed\r\n");
        return;
    }

    int one = 1;
    lwip_setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(HTTP_PORT);

    if (lwip_bind(srv, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        printf("eth_http: bind failed\r\n");
        lwip_close(srv);
        return;
    }
    if (lwip_listen(srv, 8) != 0) {
        printf("eth_http: listen failed\r\n");
        lwip_close(srv);
        return;
    }

    printf("eth_http: listening on :%d\r\n", HTTP_PORT);

    for (;;) {
        struct sockaddr_in client;
        socklen_t clen = sizeof(client);
        int c = lwip_accept(srv, (struct sockaddr *)&client, &clen);
        if (c >= 0) {
            /* Disable Nagle so lwIP sends each segment immediately, even if
             * the ACK of the previous header segment is delayed on the CH395
             * MAC-raw link. */
            int one = 1;
            lwip_setsockopt(c, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
            handle_client(c);
            lwip_close(c);
        }
    }
}

int http_server_start(void)
{
    return aos_task_new("http_server", http_server_task, NULL, 4096);
}
