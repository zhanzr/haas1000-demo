# coremark_320m — CoreMark 1.0.1 on HaaS100 @ 320 MHz

CoreMark 1.0.1 benchmark for the HaaS 100 board (HaaS1000, Cortex-M33),
ported from the STM32H723 reference (`h723-mini/app/coremark_550m`) to
AliOS Things. Uses the official CoreMark sources
(`src/coremark_1_0_1/`) with a `core_portme` that measures with the
1 kHz tick. `ITERATIONS = 25000`, performance run.

## Build

```
sh build.sh          # or: scons --board=haas100
```

Requires the vendored SDK + toolchain per the repo/board READMEs.

## Flash

Board in download mode:

```
cd D:/haas1000_prj/haas100/hardware/chip/haas1000/release/write_flash_tool
./bes_download.exe
```

## Expected output (COM port @ 1500000)

```
--- CoreMark run on HaaS100 (Cortex-M33) @ 320000000 Hz ---
CoreMark 1.0 : ...
...
CoreMark Size    : ...
CoreMark 1.0 (N) / (sec) : <value>
CoreMark 1.0 (N) : <value>
CoreMark 1.0 (N) / (MHz) : <value>
...
--- CoreMark complete. 320000000 Hz, GCC ...
```

The run repeats every 10 s. A fresh flash of the RTOS image updates the
`ota_rtos.bin` in `write_flash_tool\ota_bin\` (the build's post-image
step copies it there).
