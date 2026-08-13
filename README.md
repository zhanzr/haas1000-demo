# HaaS100 (AliOS Things) Project

AliOS Things 3.3 application development for the Aliyun **HaaS 100**
development board (HaaS1000 SoC).

```
D:\haas1000_prj\
  .gitignore
  .gitattributes                 <- Git LFS for vendored SDK binaries
  README.md                     <- this file: overall / repo-level detail
  haas100\                      <- HaaS100 board workspace (board-level detail
  │                               lives in haas100\README.md)
    .aos                        <- aos workspace marker
    VENDORED.md                 <- upstream AliOS Things commit + patches
    kernel\                     <- vendored SDK subset (real dirs, tracked)
    hardware\                   <- vendored SDK subset (tracked)
    components\                 <- vendored SDK subset (tracked)
    board_images\               <- board photos / diagrams for haas100\README.md
    README.md                   <- board-level detail
    solutions\helloworld_demo\  <- the app project (project-level detail in
                                    solutions\helloworld_demo\README.md)
```

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
2. **ARM toolchain**: a stock GNU Arm Embedded Toolchain exposed at a
   space-free path. **Default: Arm GNU Toolchain 15.3.Rel1** at
   `D:\arm-none-eabi-tc` (a junction to
   `D:\Arm\GNU Toolchain mingw-w64-x86_64-arm-none-eabi`), `bin` on the
   user PATH. The solution selects it in
   `solutions\helloworld_demo\package.yaml`:
   ```yaml
   solution:
     toolchain_prefix: arm-none-eabi
     toolchain_path: "D:/arm-none-eabi-tc"
   ```
   Fallback: the era-matched "GNU Arm Embedded Toolchain 10.3-2021.10"
   (repoint the `D:\arm-none-eabi-tc` junction to it) — this 2021 SDK
   needs no extra flags on 10.3.

## SDK patches (already applied in the vendored copy)

The vendored SDK already carries the patches needed to build with the
stock toolchain (see `haas100\VENDORED.md` for the exact list):
`spi_dev.c`/`uart_dev.c` `#include <drivers/mutex.h>`,
`libc_stub/newlib_stub.c` newlib>=4.3 `__sinit` conditional, and the
fixed Bes download configs.

Additionally, install the sysroot compat header (defines the `_POSIX_*`
macros used by `components/posix/src/enviro.c`, as a pure-`#define`
header so the linker-script step is unaffected):
`D:\arm-none-eabi-tc\arm-none-eabi\include\aos_compat.h`

The solution CFLAGS additionally suppress GCC 14/15 default-error
warnings that the 2021 SDK triggers (`-Wno-implicit-function-declaration
-Wno-implicit-int -Wno-error=incompatible-pointer-types
-Wno-error=int-conversion`); these are in
`solutions\helloworld_demo\package.yaml`.

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
cd D:/haas1000_prj/haas100/hardware/chip/haas1000/release/write_flash_tool
./bes_download.exe
```

See `haas100\README.md` for partition offsets, console baud, and USB
stability notes.
