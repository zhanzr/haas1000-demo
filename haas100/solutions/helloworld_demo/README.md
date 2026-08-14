# helloworld_demo (HaaS100 project)

AliOS Things 3.3 application for the HaaS 100 board. Runs on the
Cortex-M33 and demonstrates the board's peripherals.

## What it does

- Prints `hello world! count N` repeatedly.
- Prints the **CPU frequency** at startup and every 20 counts
  (`hal_sys_timer_calc_cpu_freq`, e.g. `320 MHz` on the Cortex-M33).
- Prints the **GPADC channels** every loop iteration (battery, chan0,
  chan2..6); the ADC-key channel is excluded (see board README).
- Sweeps the 5 on-board **LEDs** in a knight-rider pattern
  (`led_switch()`, GPIO 40/41/36/35/34).

## Files

```
helloworld.c        <- application_start: freq print, ADC print, LED sweep
maintask.c          <- aos_maintask -> board_init -> aos_components_init
k_app_config.h      <- kernel config switches
package.yaml        <- solution config (board, toolchain, deps, cflags)
SConstruct/Makefile <- build entry (aostools scons)
build.sh            <- one-command build helper (sh build.sh)
posix note:         <- see root README (SDK patches)
```

## Build

```
sh build.sh          # or: scons --board=haas100
```

Prerequisites (SDK junctions, toolchain, `aos-tools`, SDK patches) are
described in the repo README (`..\..\..\README.md`). Board details are in
the board README (`..\..\README.md`).

Outputs:

```
out\helloworld_demo@haas100.bin   <- app image
out\helloworld_demo@haas100.elf   <- ELF for gdb (SWD)
```

## Flash

Build first (`./build.sh`), put the board in **download mode** (hold
Download while powering on), then run from this directory:

```
./flash.sh
```

`flash.sh` stages this project's image and calls the Bes download tool
(no manual path switching). Board-specific flash detail (download mode,
partition offsets) is in the board README (`..\..\README.md`).

## Expected output (COM port @ 1500000)

```
             Welcome to AliOS Things
nano entry here!
arch=arm cpu=CORTEX-M33, current CPU frequency = 320000000 Hz (320.0 MHz)
internal ADC (GPADC) channels:
  chan0     = 1270 mV
  battery   = 1274 mV
  chan2     = 66 mV
  chan3     = 45 mV
  chan4     = 58 mV
  chan5     = 6 mV
  chan6     = 6 mV
hello world! count 0
hello world! count 1
...
```
