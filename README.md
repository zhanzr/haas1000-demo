# HaaS100 (HaaS1000) AliOS Things project

AliOS Things 3.3 application for the Aliyun **HaaS 100** development
board (HaaS1000 SoC, dual Cortex-M33).

## Layout

```
D:\haas1000_prj\
  .gitignore
  README.md
  haas100\                        <- aos workspace root
    .aos                          <- workspace marker
    kernel\      -> D:\AliOS-Things\kernel      (junction, ignored)
    hardware\    -> D:\AliOS-Things\hardware    (junction, ignored)
    components\  -> D:\AliOS-Things\components  (junction, ignored)
    solutions\helloworld_demo\    <- the app (tracked)
    flash_tool\                   <- fixed Bes download configs (tracked)
```

The three `kernel`/`hardware`/`components` folders are **directory
junctions** to the AliOS Things 3.3 SDK at `D:\AliOS-Things` and are
ignored by git. The SDK sources live in one place; the project only
tracks its own app + configuration.

## Prerequisites / one-time setup

1. **AliOS Things 3.3 SDK** at `D:\AliOS-Things` (GitHub
   `alibaba/AliOS-Things`). Create the junctions:
   ```
   mklink /J D:\haas1000_prj\haas100\kernel     D:\AliOS-Things\kernel
   mklink /J D:\haas1000_prj\haas100\hardware   D:\AliOS-Things\hardware
   mklink /J D:\haas1000_prj\haas100\components D:\AliOS-Things\components
   ```
2. **Build tools**: Python 3 + `pip install aos-tools` (provides `aos`
   and the `aostools` scons module).
3. **ARM toolchain**: a stock GNU Arm Embedded Toolchain, exposed at a
   space-free path. On this machine `D:\arm-none-eabi-tc` is a junction
   to the installed "GNU Arm Embedded Toolchain 10 2021.10", and its
   `bin` is on the user PATH. The solution selects it in
   `solutions/helloworld_demo/package.yaml`:
   ```yaml
   solution:
     toolchain_prefix: arm-none-eabi
     toolchain_path: "D:/arm-none-eabi-tc"
   ```

## SDK patches (required to build)

The stock toolchain newlib differs from the original Alibaba
`arm-ali-aoseabi` toolchain, so two SDK files need `#include
<drivers/mutex.h>` (the AliOS "base" VFS layer uses `struct mutex`
without including it):

- `hardware/chip/haas1000/drivers/.../spi/src/spi_dev.c`
- `hardware/chip/haas1000/drivers/.../uart/src/uart_dev.c`

Also install the sysroot compat header (defines the `_POSIX_*` macros
used by `components/posix/src/enviro.c`, as a pure-`#define` header so
the linker-script step is unaffected):
`D:\arm-none-eabi-tc\arm-none-eabi\include\aos_compat.h`

## Build

```
cd haas100\solutions\helloworld_demo
build.bat            # or: scons --board=haas100
```

Outputs: `out\helloworld_demo@haas100.bin` (app image),
`out\helloworld_demo@haas100.elf` (debug).

## Flash

The Bes download tool reads `bes_dld_cfg.yaml` from its own directory.
The fixed configs/helpers are versioned in `haas100\flash_tool\`; copy
them into the SDK's tool folder (overwriting the broken originals):

```
copy haas100\flash_tool\* D:\AliOS-Things\hardware\chip\haas1000\release\write_flash_tool\
cd /d D:\AliOS-Things\hardware\chip\haas1000\release\write_flash_tool
bes_download.exe      # board in download mode (hold Download on power-on)
```

The default config flashes only the boot-critical images + RTOS (fast).
`flash_full.bat` also writes the filesystem/optional regions;
`flash_littlefs.bat` writes only the filesystem. See
`solutions/helloworld_demo/README.md` for all details (offsets, USB
stability, console baud).
