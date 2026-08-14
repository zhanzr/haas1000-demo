# HaaS100 (AliOS Things) Project

AliOS Things 3.3 application development for the Aliyun **HaaS 100**
development board (HaaS1000 SoC).

Layout at a glance:

- `haas100/` — the HaaS100 board workspace.
  - `haas100/kernel`, `haas100/hardware`, `haas100/components` — the
    **vendored** AliOS Things SDK subset (HaaS1000-only, tracked in this
    repo; upstream commit + patches in `haas100/VENDORED.md`).
  - `haas100/solutions/` — the applications: `helloworld_demo`,
    `dhry_320m`, `coremark_320m`, `eth_http` (Ethernet + web server),
    `wifi_http` (Wi-Fi + web server). Each has its own README.
  - `haas100/board_images/` — board photos embedded in `haas100/README.md`.
  - `haas100/README.md` — board-level detail (hardware, flash).
  - `haas100/.aos` — aos workspace marker.

The `e_server/` directory at the repo root is the **standalone web app**
(source of truth for the site, adapted for the HaaS100: 5 LEDs, first 3
GPADC channels). `eth_http` embeds its generated `web_assets.h` and
mirrors its API on the board. `e_server` is kept for future changes.

Three README levels, per request:
- **Repo / overall** — this file: how the pieces fit together, prerequisites,
  SDK setup, build & flash overview.
- **Board level** — `haas100\README.md`: hardware of the HaaS100 board
  (LEDs, ADC, UART, flash layout), with images in `haas100\board_images\`.
- **Project level** — `haas100\solutions\helloworld_demo\README.md`: the
  `helloworld_demo` application itself (what it does, how to build it,
  expected output).

## SDK: vendored AliOS Things subset

The AliOS Things 3.3 SDK (abandoned upstream) is **vendored** into this
repo as a HaaS1000-only subset under `haas100\{kernel,hardware,components}`
— it is tracked here so the repo builds independently. Upstream commit
**`a99f20706`** (2023-07-04) and the applied patches are recorded in
`haas100\VENDORED.md`.

## Prerequisites / one-time setup

0. **Git LFS** (the vendored SDK binaries are stored via Git LFS). After
   cloning: `git lfs install && git lfs pull`.
1. **Build tools**: Python 3 + `pip install aos-tools` (provides the `aos`
   command and the `aostools` scons module).
2. **ARM toolchain**: the **Arm GNU Toolchain 15.3.1** (`arm-none-eabi`),
   exposed at a **space-free path**. Each `solutions/*/package.yaml`
   selects it via `toolchain_prefix: arm-none-eabi` and
   `toolchain_path: <space-free path to the toolchain root>` (on this
   machine a junction gives a space-free path). The toolchain `bin` is
   on the user PATH. GCC 14/15 default-error warnings are suppressed in
   the solution CFLAGS (see below).

## SDK patches (already applied in the vendored copy)

The vendored SDK already carries the patches needed to build with the
stock toolchain (see `haas100\VENDORED.md` for the exact list):
`spi_dev.c`/`uart_dev.c` `#include <drivers/mutex.h>`,
`libc_stub/newlib_stub.c` newlib>=4.3 `__sinit` conditional, and the
fixed Bes download configs.

Additionally, install the sysroot compat header (defines the `_POSIX_*`
macros used by `components/posix/src/enviro.c`, as a pure-`#define`
header so the linker-script step is unaffected), into the toolchain
sysroot: `<toolchain>/arm-none-eabi/include/aos_compat.h`.

The solution CFLAGS additionally suppress GCC 14/15 default-error
warnings that the 2021 SDK triggers (`-Wno-implicit-function-declaration
-Wno-implicit-int -Wno-error=incompatible-pointer-types
-Wno-error=int-conversion`); these are in each `solutions/*/package.yaml`.

## Build

```
cd haas100\solutions\helloworld_demo
sh build.sh          # or: scons --board=haas100
```

Outputs: `out\helloworld_demo@haas100.bin` (app image),
`out\helloworld_demo@haas100.elf` (debug).

## Flash

Board + flash detail is in `haas100\README.md`. In short: the fixed Bes
download configs are vendored in
`haas100\hardware\chip\haas1000\release\write_flash_tool\`. With the
board in download mode (hold **Download** on power-on):

```
cd haas100/hardware/chip/haas1000/release/write_flash_tool
./bes_download.exe
```

See `haas100\README.md` for partition offsets, console baud, and USB
stability notes.
