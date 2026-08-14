# wifi_http — HaaS100 Wi-Fi + embedded web server

Connects the HaaS 100 to a Wi-Fi access point via the AliOS `netmgr` /
`bwifi` stack, gets an IP by DHCP, and serves the e_server single-page app
(see `../../../../e_server/`) on **port 80** using lwIP sockets.

> **Wi-Fi status (2026-08-15): NOT CONNECTED - needs debug.**
> `netmgr_wifi_connect()` first issues a scan ioctl (`START_SCAN` /
> `START_SPECIFIED_SCAN`) which hangs inside the **prebuilt**
> `bwifi_config_scan()` in `libmcuhaas1000.a` (closed binary, no source).
> The driver's actual connect (`haas1000_wifi_connect` →
> `wifi_connect_task` → `bwifi_connect_to_ssid`) does **not** need a scan,
> so `app_main.c` calls the driver `WIFI_DEV_CMD_CONNECT` ioctl directly
> and lets the netmgr event pipeline run netif-up + DHCP
> (`EVENT_WIFI_CONNECTED` → `CONN_STATE_CONNECTED` → `wifi_obtaining_ip` →
> `wifi_dhcp_start()` → `EVENT_NETMGR_DHCP_SUCCESS`).
>
> **Board test result (2026-08-15, direct-ioctl build):** the connect ioctl
> returns, the WiFi HAL is up (`wifi init success!!`), but the association
> never completes — serial shows `handshake failed 1 times`
> (`EVENT_WIFI_HANDSHAKE_FAILED`, event 4110) and the app reports
> `LAN IP 127.0.0.1` (loopback, no DHCP lease). The prebuilt
> `bwifi_connect_to_ssid()` association/handshake is failing on this board;
> root-causing it needs the closed `libmcuhaas1000.a` behavior traced
> (or a WiFi-stack log level bump in `package.yaml`). See the status block
> in this README's git history for the exact serial log.

Page/API (same as `eth_http`, adapted for HaaS100):
- **LED tab** — controls the 5 on-board LEDs (GPIO 40/41/36/35/34).
- **ADC tab** — plots the first 3 GPADC channels: `chan0`, `battery`, `chan2` (mV).
- **Board info tab** — arch + LAN IP.

## Configure your Wi-Fi

Edit `src/wifi_config.h` (or copy `src/wifi_config.h.example` to
`src/wifi_config.h` and fill in your credentials):

```c
#define DEFAULT_AP      "your_ssid"
#define DEFAULT_PASSWD  "your_password"
```

Then rebuild.

## Build

```
sh build.sh          # or: scons --board=haas100
```

## Flash & run

**Important**: `netmgr_wifi` stores its config on the **`/data` (littlefs)
filesystem**. If you previously flashed only the minimal image set (no
littlefs), `/data` is not mounted and Wi-Fi config fails ("no config
found"). Flash the **full** image set (includes the littlefs filesystem):

```
cd ../../hardware/chip/haas1000/release/write_flash_tool
./flash_full.sh        # board in download mode: boot + RTOS + littlefs + ...
```

Then power the board normally. Open a serial monitor at **1500000** and watch:

```
wifi_http: connecting to '<ssid>'
wifi_http: LAN IP 192.168.x.x
wifi_http: open http://192.168.x.x/
```

Open that URL in a browser (same Wi-Fi network).

## Notes

- Requires the Wi-Fi antenna; the CH395Q Ethernet netif is skipped at boot
  (`CONFIG_ETH_LWIP_APP_INIT`), and `AOS_COMP_WIFI` enables the Wi-Fi HAL
  + `netmgr_wifi`.
- lwIP is started with `tcpip_init()` before the Wi-Fi netif is created, so
  sockets/DHCP work over the Wi-Fi interface.
- On success the AP is auto-saved; on next boot the app auto-reconnects to
  the last-used AP (`netmgr_wifi_set_auto_save_ap`).
