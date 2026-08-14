# e_server - embedded web demo (frontend + reference C backend)

A minimal single-page web app (originally written for an MCU board), plus a
host-side C backend that mirrors the API the board's embedded server will
implement. The web assets are bundled into C arrays by a build script
(inline CSS/JS, gzip the page, embed the images raw) so they can be served
straight from flash on the MCU.

## Layout

```
e_server/
  web/             raw frontend sources
    index.html     single page, three tabs
    style.css
    app.js
  public/          images served to the page (embedded into web_assets.h)
    board_0.png
    board_1.png
    board_2.png
  build_web.py     bundle: inline CSS/JS -> gzip page -> C arrays (--out to reuse)
  web_assets.h     generated C arrays (page + images + lookup table)
  server.c         reference C backend (host-side)
  Makefile / build.sh
```

`e_server/` is the **standalone source of truth for the site**. Other boards
and projects embed it by pointing the packer at their own output path, e.g.
`python build_web.py --out <project>/Inc/web_assets.h`.

## Frontend (three tabs, no external libraries)

1. **LED control** - checkboxes reflecting the board LEDs (5 on the HaaS100).
   Each click POSTs the new state to the backend (no page reload) and the
   checkboxes re-sync to the server's reply. Rapid clicks are coalesced.
2. **ADC values** - three canvas plots (the first 3 GPADC channels: `chan0`, `battery`, `chan2`, in mV).
   A dropdown picks the sample interval: 1 s (default), 2 s, 4 s; the choice
   is persisted in `localStorage`. Samples are polled only while the tab is
   open; on timeout/error the previous value is kept and the series continues.
3. **Board info** - architecture, LAN IP, public IP, geo location and weather
   (each falls back to "N/A" if unavailable), a manual *Refresh* button, and
   the board photos from `public/`.

## API (shared contract with the embedded server)

| Route                | Description                                     |
| -------------------- | ----------------------------------------------- |
| `GET /`              | the page: gzipped, CSS/JS inlined (`Content-Encoding: gzip`) |
| `GET /api/leds`      | `{"leds":[0,1,0]}`                              |
| `POST /api/leds`     | body `{"leds":[0,1,0]}` -> applied, `{"leds":[...]}` |
| `GET /api/adc`       | `{"ch0_mv":1270,"bat_mv":1274,"ch2_mv":66,"ts":...}` |
| `GET /api/info`      | `{"arch","lan_ip","public_ip","geo","weather","ts"}` (nulls = unavailable) |
| `GET /public/*`      | raw image bytes (`image/jpeg`) from `embedded_files[]` |

On the host the reference backend reads real ADC-style values from a
simulation and fetches the public IP / geo / weather over plain HTTP
(api.ipify.org, ip-api.com, wttr.in) - best-effort, so offline they come back
as `null` and the page shows "N/A". On the board those fields would come from
an HTTP client (+ TLS for HTTPS upstreams).

## Build & run (host reference)

Needs Python 3 and a host C compiler (gcc/clang). Windows/MinGW adds
`-lws2_32` automatically.

```bash
python build_web.py      # or: make web_assets.h
make                     # or: bash build.sh
./e_server 8080          # or: make run
```

Then open `http://localhost:8080/`.

## Bundling (build_web.py)

```text
[ index.html ] ──┬── inline <style> + <script> ──► index.html ── gzip ──► index_html_gz[]
[ style.css  ] ──┘                                          (raw)        │
[ app.js     ] ──┘                                                          ▼
[ public/*.jpg ] ─────────────────────────► raw byte arrays + embedded_files[]  web_assets.h
```

* The page is served gzipped (browser decompresses via `Content-Encoding: gzip`).
* JPEG/PNG are already compressed, so the images are embedded **raw** with a
  `path -> {ctype, data, len}` lookup table the server routes against.
* Regenerate after any change to `web/` or `public/`; commit `web_assets.h`
  alongside the sources.
* Other projects reuse the packer:
  `python build_web.py --out ../some/project/Inc/web_assets.h`.

## Porting to the board

`web_assets.h` drops straight into flash; `server.c`'s routing is the blueprint
for an embedded server. **Already done** in this repo's HaaS100 projects
(`haas100/solutions/eth_http` over Ethernet, `haas100/solutions/wifi_http`
over Wi-Fi): their `build.sh` regenerates `src/web_assets.h` with
`python e_server/build_web.py --out src/web_assets.h`, and their lwIP-socket
server (`src/http_server.c`) serves this exact site and API - `/api/leds` and
`/api/adc` read the real on-board LEDs and GPADC channels, `/api/info`
reports `arch` + `lan_ip` with `public_ip`/`geo`/`weather` as `null` (the
board has no HTTP/TLS client yet).
