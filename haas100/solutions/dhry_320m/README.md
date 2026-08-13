# dhry_320m — Dhrystone 2.1 on HaaS100 @ 320 MHz

Dhrystone 2.1 benchmark for the HaaS 100 board (HaaS1000, Cortex-M33),
ported from the STM32H723 reference (`h723-mini/app/dhry_550m`) to
AliOS Things. Runs `RUN_NUMBER` (12 M) loops, measures with the 1 kHz
tick, and reports Dhrystones/sec and DMIPS/MHz.

## Build

```
build.bat            # or: scons --board=haas100
```

Requires the vendored SDK + toolchain per the repo/board READMEs.

## Flash

Board in download mode:

```
cd /d D:\haas1000_prj\haas100\hardware\chip\haas1000\release\write_flash_tool
bes_download.exe
```

## Expected output (COM port @ 1500000)

```
=== Dhrystone 2.1 on HaaS100 (Cortex-M33) @ 320000000 Hz ===
Dhrystone Benchmark, Version 2.1 (Language: C)
...
Dhrystones per Second:  <value>
DMIPS/MHz:              <value>
```

The run repeats every 10 s. A fresh flash of the RTOS image updates the
`ota_rtos.bin` in `write_flash_tool\ota_bin\` (the build's post-image
step copies it there).
