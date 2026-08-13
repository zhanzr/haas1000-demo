# HaaS100 (AliOS Things) Project

AliOS Things 3.3 application development for the Aliyun **HaaS 100**
development board (HaaS1000 SoC).

```
D:\haas1000_prj\
  .gitignore
  README.md                     <- this file: overall / repo-level detail
  haas100\                      <- HaaS100 board workspace (board-level detail
  │                               lives in haas100\README.md)
    .aos                        <- aos workspace marker
    kernel\   -> D:\AliOS-Things\kernel      (junction, ignored)
    hardware\ -> D:\AliOS-Things\hardware    (junction, ignored)
    components\-> D:\AliOS-Things\components (junction, ignored)
    board_images\               <- board photos / diagrams for haas100\README.md
    flash_tool\                 <- fixed Bes download configs (versioned)
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

## Prerequisites / one-time setup

1. **AliOS Things 3.3 SDK** at `D:\AliOS-Things` (GitHub
   `alibaba/AliOS-Things`). The board workspace links to it through
   directory junctions:
   ```
   mklink /J D:\haas1000_prj\haas100\kernel     D:\AliOS-Things\kernel
   mklink /J D:\haas1000_prj\haas100\hardware   D:\AliOS-Things\hardware
   mklink /J D:\haas1000_prj\haas100\components D:\AliOS-Things\components
   ```
   The junctions are ignored by git; the SDK itself is not part of this
   repository.
2. **Build tools**: Python 3 + `pip install aos-tools` (provides the `aos`
   command and the `aostools` scons module).
3. **ARM toolchain**: a stock GNU Arm Embedded Toolchain exposed at a
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

## SDK patches (required to build)

The stock toolchain newlib differs from the original Alibaba
`arm-ali-aoseabi` toolchain, so these SDK files are patched:

- `hardware/chip/haas1000/drivers/.../spi/src/spi_dev.c` and
  `.../uart/src/uart_dev.c` — add `#include <drivers/mutex.h>` (the
  AliOS "base" VFS layer uses `struct mutex` without including it).
- `components/libc_stub/newlib_stub.c` — the `__sinit`/`__sdidinit`
  bootstrap is wrapped in
  `#if (__NEWLIB__ == 4 && __NEWLIB_MINOR__ >= 3) || (__NEWLIB__ > 4)`
  because newlib >= 4.3 removed those internals (`_REENT_INIT(_PTR)`
  fully initializes stdio now). This keeps both the 4.1 (GCC 10.3) and
  4.6 (GCC 15.3) newlib builds working.

Also install the sysroot compat header (defines the `_POSIX_*` macros
used by `components/posix/src/enviro.c`, as a pure-`#define` header so
the linker-script step is unaffected):
`D:\arm-none-eabi-tc\arm-none-eabi\include\aos_compat.h`

The solution CFLAGS additionally suppress GCC 14/15 default-error
warnings that the 2021 SDK triggers (`-Wno-implicit-function-declaration
-Wno-implicit-int -Wno-error=incompatible-pointer-types
-Wno-error=int-conversion`); these are in
`solutions\helloworld_demo\package.yaml`.

## Build

```
cd haas100\solutions\helloworld_demo
build.bat            # or: scons --board=haas100
```

Outputs: `out\helloworld_demo@haas100.bin` (app image),
`out\helloworld_demo@haas100.elf` (debug).

## Flash

Board + flash detail is in `haas100\README.md`. In short: the Bes
download tool reads `bes_dld_cfg.yaml` from its own directory; the fixed
configs/helpers are versioned in `haas100\flash_tool\`. Copy them into
the SDK tool folder, then:

```
cd /d D:\AliOS-Things\hardware\chip\haas1000\release\write_flash_tool
bes_download.exe      # board in download mode (hold Download on power-on)
```

See `haas100\README.md` for partition offsets, console baud, and USB
stability notes.
