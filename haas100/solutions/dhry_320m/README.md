# dhry_320m — Dhrystone 2.1 on HaaS100 @ 320 MHz

Dhrystone 2.1 benchmark for the HaaS 100 board (HaaS1000, Cortex-M33) on
AliOS Things. Runs `RUN_NUMBER` (12 M) loops, measures with the 1 kHz
tick, and reports Dhrystones/sec and DMIPS/MHz.

## Build

```
sh build.sh          # or: scons --board=haas100
```

Requires the vendored SDK + toolchain per the repo/board READMEs.

## Flash

Build first (`./build.sh`), put the board in **download mode** (hold
Download while powering on), then run from this directory:

```
./flash.sh
```

`flash.sh` stages this project's image and calls the Bes download tool
(no manual path switching).

The run repeats every 10 s. A fresh flash of the RTOS image updates the
`ota_rtos.bin` in `write_flash_tool/ota_bin/` (the build's post-image
step copies it there).

## Measured results

Recorded on this board with GCC 15.3.1 (`-Ofast -ffp-contract=fast
-funroll-loops`):

```
MicroSecond for one run through Dhrystone[29189-44275]:  1.257
Dhrystones per Second:  795439.500
DMIPS/MHz:      1.415

CPU freq: 320000000 Hz (320 MHz)
Compiler: GCC 15.3.1 20260627
```
