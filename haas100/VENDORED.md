# Vendored AliOS Things SDK (HaaS1000 subset)

This directory contains a **vendored subset** of the upstream AliOS Things
SDK, trimmed to what is needed to build a HaaS 100 (HaaS1000) RTOS
application (e.g. `solutions/helloworld_demo`). Aliyun has stopped
maintaining the upstream repository, so the needed parts are committed
here to keep this repo self-contained and buildable independently.

## Upstream source

- Repository: https://github.com/alibaba/AliOS-Things
- Vendored commit: **`a99f20706f9c666903a12a205edce13263b1fadb`**
- Commit date: 2023-07-04
- Branch: master

You can verify the current checkout against upstream with:

```
git -C <upstream-repo> log -1 a99f20706f9c666903a12a205edce13263b1fadb
```

## What was vendored

Only the dependency closure for `helloworld_demo@haas100` was copied
(tracked files only, at the commit above). Structure:

```
kernel/rhino                     RTOS kernel
hardware/arch/armv8m             Cortex-M33 arch support
hardware/chip/haas1000           HaaS1000 SoC BSP (prebuilt libs, drivers, flash tools)
hardware/board/haas100           HaaS100 board BSP
components/
  cli csi init kv libc_stub lwip netmgr osal_aos posix select sntp
  ulog uservice vfs
  drivers/core/base
  drivers/peripheral/{adc,flash,gpio,i2c,pwm,spi,timer,uart,watchdog}
```

Everything else in the upstream repo (other boards/chips, esp-idf,
py_engine, audio libs, documentation, ...) was **not** vendored.

## Patches applied on top of the upstream commit

These changes are required to build with the stock ARM GNU toolchain
(the original `arm-ali-aoseabi` toolchain is unavailable):

| File | Change |
|------|--------|
| `components/drivers/peripheral/spi/src/spi_dev.c` | added `#include <drivers/mutex.h>` |
| `components/drivers/peripheral/uart/src/uart_dev.c` | added `#include <drivers/mutex.h>` |
| `components/libc_stub/newlib_stub.c` | `__sinit`/`__sdidinit` bootstrap wrapped in `#if (__NEWLIB__==4 && __NEWLIB_MINOR__>=3) \|\| (__NEWLIB__>4)` (newlib >= 4.3 removed those internals) |
| `hardware/chip/haas1000/release/write_flash_tool/bes_dld_cfg.yaml` | fixed COM/baud/flash offsets; minimal image set (boot + RTOS only) |
| `.../write_flash_tool/bes_dld_cfg_full.yaml` | full image set (adds littlefs/boot1_sec/factory/pub_otp) |
| `.../write_flash_tool/bes_dld_cfg_littlefs.yaml` | filesystem-only config |
| `.../write_flash_tool/flash_full.bat` | helper: flash full image set |
| `.../write_flash_tool/flash_littlefs.bat` | helper: flash filesystem only |

The toolchain sysroot header `D:\arm-none-eabi-tc\arm-none-eabi\include\aos_compat.h`
(installed into the toolchain, not the SDK) is documented in the repo
README and provides the `_POSIX_*` macros used by `components/posix`.

## Generated files (not vendored)

The following are produced by the build and are git-ignored; a fresh
clone regenerates them:

- `hardware/chip/haas1000/macro_defines.h` (linker-script prebuild)
- `hardware/chip/haas1000/_haas1000_alios.lds` (linker script)
- `haas100/**/SConscript` (aostools temporary build scripts)
- `hardware/chip/haas1000/release/release_bin/`, `.../write_flash_tool/ota_bin/`,
  `.../write_flash_gui/ota_bin/`, `prebuild/littlefs.bin` (post-build image copies)

## Syncing future upstream changes

To pull upstream changes into the vendored subset:

```
git -C <upstream> fetch origin
git -C <upstream> diff a99f20706f9c666903a12a205edce13263b1fadb origin/master \
    -- kernel/rhino hardware/arch/armv8m hardware/chip/haas1000 \
    hardware/board/haas100 components/cli components/csi \
    components/libc_stub components/init components/kv components/lwip \
    components/netmgr components/osal_aos components/posix \
    components/select components/sntp components/ulog components/uservice \
    components/vfs components/drivers/core/base \
    components/drivers/peripheral/{adc,flash,gpio,i2c,pwm,spi,timer,uart,watchdog}
```

Then re-apply the patches above as needed.
