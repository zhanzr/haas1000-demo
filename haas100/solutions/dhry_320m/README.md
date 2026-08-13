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

Board in download mode (run from the repository root):

```
cd haas100/hardware/chip/haas1000/release/write_flash_tool
./bes_download.exe
```

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
