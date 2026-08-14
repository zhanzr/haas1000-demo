# coremark_320m — CoreMark 1.0.1 on HaaS100 @ 320 MHz

CoreMark 1.0.1 benchmark for the HaaS 100 board (HaaS1000, Cortex-M33)
on AliOS Things. Uses the official CoreMark sources
(`src/coremark_1_0_1/`) with a `core_portme` that measures with the
1 kHz tick. `ITERATIONS = 25000`, performance run.

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
--- CoreMark run on HaaS100 (Cortex-M33) @ 320000000 Hz ---
2K performance run parameters for coremark.
CoreMark Size    : 666
Total ticks      : 26618
Total time (secs): 26.618000
Iterations/Sec   : 939.214066
Iterations       : 25000
Compiler version : GCC 15.3.1 20260627
Compiler flags   : -Ofast -funroll-loops
Memory location  : Static
seedcrc          : 0xe9f5
[0]crclist       : 0xe714
[0]crcmatrix     : 0x1fd7
[0]crcstate      : 0x8e3a
[0]crcfinal      : 0xcc42
Correct operation validated. See readme.txt for run and reporting rules.
CoreMark 1.0 : 939.214066 / GCC 15.3.1 20260627 -Ofast -funroll-loops / Static
--- CoreMark complete. 320000000 Hz, GCC 15.3.1 20260627 ---
```
