# HaaS100 helloworld_demo (LED blink)

AliOS Things 3.3 application for the Aliyun **HaaS 100** board (HaaS1000
SoC, Cortex-M33). Prints `hello world!` over UART and blinks the five
on-board LEDs in a knight-rider sweep.

## LED GPIO mapping (as defined by the AliOS board BSP)

From `hardware/board/haas100/drivers/led.c` and
`hardware/chip/haas1000/.../hal_iomux_haas1000.h` (pin index == GPIO
number, no offset):

| LED  | AliOS pin   | GPIO |
|------|-------------|------|
| LED1 | `HAL_IOMUX_PIN_LED1` | 40 |
| LED2 | `HAL_IOMUX_PIN_LED2` | 41 |
| LED3 | `HAL_IOMUX_PIN_P4_4` | 36 |
| LED4 | `HAL_IOMUX_PIN_P4_3` | 35 |
| LED5 | `HAL_IOMUX_PIN_P4_2` | 34 |

LEDs are active-low (`led_switch(id, LED_ON)` drives the pin low). The app
uses the board's `led_switch()` API (`led.h`), so it stays correct even if
a board revision remaps the pins.

## Layout

```
haas100\                       <- aos workspace root (this project)
  .aos                         <- workspace marker
  kernel\      -> junction to D:\AliOS-Things\kernel
  hardware\    -> junction to D:\AliOS-Things\hardware
  components\  -> junction to D:\AliOS-Things\components
  solutions\helloworld_demo\   <- this solution
```

The workspace references the AliOS Things 3.3 SDK at `D:\AliOS-Things`
through directory junctions, so the SDK sources stay in one place.

## Toolchain

The original `arm-ali-aoseabi` toolchain (distributed by Aliyun, repo on
Gitee) is currently not downloadable (Gitee LFS is broken/slow). The chip
is a plain Cortex-M33/ARMv8-M, so the stock **GNU Arm Embedded Toolchain
10.3-2021.10** already installed on this machine is used instead.

A space-free junction makes it usable by the SDK (the build builds shell
command lines with the toolchain path, so spaces are not allowed):

```
D:\arm-none-eabi-tc  ->  D:\GNU Arm Embedded Toolchain\10 2021.10
```

The solution selects it in `package.yaml`:

```yaml
solution:
  toolchain_prefix: arm-none-eabi
  toolchain_path: "D:/arm-none-eabi-tc"
```

`D:\arm-none-eabi-tc\bin` is added to the user PATH.

### Compatibility shim (sysroot header)

The Alibaba toolchain shipped a customized newlib that additionally
provided POSIX feature macros and a bare `struct mutex` used by the SDK's
VFS driver layer. The stock toolchain does not. Two small fixes were made:

1. `D:\arm-none-eabi-tc\arm-none-eabi\include\aos_compat.h` — defines the
   `_POSIX_*` macros used by `components/posix/src/enviro.c`. It is a
   pure-`#define` header (no `#include`, no typedefs) so it can be
   force-included via the solution CFLAGS (`-include aos_compat.h`)
   without corrupting the linker-script generation step.
2. `components/drivers/peripheral/spi/src/spi_dev.c` and
   `components/drivers/peripheral/uart/src/uart_dev.c` — added
   `#include <drivers/mutex.h>` (the definition lives in the `base`
   driver component; those two files used `struct mutex` without ever
   including it).

`poll.h` is provided by the SDK `posix` component (added to the solution
`depends`).

## Build

One-time setup:

```
pip install aos-tools          # provides 'aos' and the aostools scons module
```

Build (from this directory):

```
build.bat
```

or directly:

```
set PATH=%PATH%;D:\arm-none-eabi-tc\bin
scons --board=haas100
```

Outputs:

```
out\helloworld_demo@haas100.bin   <- app image (888 KB)
out\helloworld_demo@haas100.elf   <- ELF for gdb (SWD debug)
```

The post-build step also assembles the full flash image set under
`..\..\hardware\chip\haas1000\release\release_bin\` (bootloaders +
`ota_rtos.bin`).

## Flashing

The board enumerates as a CP2102 serial port (UART0, also the debug
console). Flashing is done over that port:

1. Hold the board **Download/boot** button, connect USB, release after
   power-on (board enters download mode).
2. Run the bundled Windows tool **from its own directory** (it uses
   relative paths and reads `bes_dld_cfg.yaml` from the CWD):

```
cd /d D:\AliOS-Things\hardware\chip\haas1000\release\write_flash_tool
bes_download.exe
```

   The `bes_dld_cfg.yaml` is already fixed for this board:
   - `COM` set to the board's port (edit if different),
   - `ota_boot2b.bin` entry removed (that file does not exist),
   - **no `remap-both`** — this board's flash rejects the flash-remap
     vendor command (`FLASH_CMD_ENABLE_REMAP error 0xff`), so all
     `--remap`/`--remap-both` flags are removed,
   - **correct flash offsets** matching the SDK partition table
     (`hardware/board/haas100/config/partition_conf.c`, block = 4 KiB):
     boot1 @ `0x2C000000`, boot_info @ `0x2C010000`, boot2a @
     `0x2C012000`, RTOS @ `0x2C02A000`, littlefs @ `0x2CB32000`,
     boot1_sec @ `0x2CFE0000`, factory @ `0x2CFFE000`, pub_otp @ `0x0`.
     (The vendor CLI config shipped with the SDK had these `ADDR` values
     commented out, and the commented legacy offsets were wrong — boot1
     then read the boot-info region and found the misplaced boot2a image,
     failing with `ota_get_bootinfo ... update_link` errors.)
   - **minimal image set** — the default `bes_dld_cfg.yaml` only flashes
     the boot-critical images + RTOS (boot1, boot_info, boot2a, RTOS).
     The filesystem and optional regions are omitted so the transfer is
     small and fast (no 4.9 MB `littlefs.bin`, so USB drops are unlikely):
       - `littlefs.bin` (filesystem `/data`) — not needed unless you use
         file/KV storage;
       - `ota_boot1_sec.bin` (secure-boot ROM) — not needed for non-secure
         boot (`security_en:0`);
       - `factory.bin` (factory data) — the original factory region stays
         intact since it is not erased;
       - `pub_otp.bin` (security OTP register) — not needed for non-secure.
     To restore them, run `flash_full.bat` (full image incl. littlefs) or
     `flash_littlefs.bat` (filesystem only). These helpers back up
     `bes_dld_cfg.yaml`, install the wider config, run the tool, and
     restore the minimal config afterwards. The full config is kept as
     `bes_dld_cfg_full.yaml`.
   - `BAUT_RATE: 921600` — 2 Mbaud caused USB/CP2102 drops (error 31,
     "device not functioning") mid-transfer,
   - `WAIT_PRESS_ENTER: OFF`.

   If a transfer fails with "device not functioning" (error 31),
   **unplug and reconnect the USB cable** (resets the CP2102), re-enter
   download mode, and re-run the tool. Error 31 is caused by the USB
   link dropping mid-transfer (usually during the 4.9 MB `littlefs.bin`);
   to reduce it, disable **USB selective suspend** in Windows power
   options, update the Silicon Labs CP210x driver, and use a rear USB 2.0
   port. A partial flash is safe: the boot-critical images (boot1,
   boot_info, boot2a, RTOS) flash first, so the board boots even if the
   trailing littlefs/factory/pub_otp are missing (only `/data` won't
   mount). Finish those with `flash_littlefs.bat`.

   Note: if you see a `NameError: wait_press_enter`, you ran it from
   the wrong directory — the tool opens `bes_dld_cfg.yaml` relative to
   the CWD.

or `aos burn -b haas100` (uses
`hardware/chip/haas1000/release/aos_burn_tool/flash_program.py`, which
needs `pyserial`).

After boot, open the same serial port at **1500000 baud** — the HaaS100
UART0 console is configured at 1.5 Mbaud in
`hardware/board/haas100/config/board.c` (the prebuilt bootloaders also
print at 1.5 Mbaud, so all output is consistent at this rate).

Expected output:
```
             Welcome to AliOS Things
nano entry here!
arch=arm cpu=CORTEX-M33, current CPU frequency = 320000000 Hz (320.0 MHz)
hello world! count 0
...
```

Note: the reported **320 MHz** is the actual Cortex-M33 PLL clock set by
the bootloaders (300 MHz is the nominal spec; boot1/boot2a report the
same value). AliOS runs entirely on the **Cortex-M33** — the HaaS1000's
dual Cortex-A7 is a separate subsystem, disabled in this build
(`CONFIG_A7_DSP_ENABLE=0`). The app prints the frequency again every 20
counts so CPU clock scaling is visible.
