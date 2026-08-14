#!/bin/sh
# Flash this project's firmware to the HaaS100 board.
# Board must be in download mode (hold Download button while powering on).
# Run from this project directory - no manual path switching needed.
set -e
cd "$(dirname "$0")"
SOL="$(basename "$(pwd)")"
BIN="out/${SOL}@haas100.bin"
TOOL_DIR="../../hardware/chip/haas1000/release/write_flash_tool"

if [ ! -f "$BIN" ]; then
    echo "error: $BIN not found - run ./build.sh first" >&2
    exit 1
fi

# stage this project's image as the OTA RTOS image, then flash
mkdir -p "$TOOL_DIR/ota_bin"
cp "$BIN" "$TOOL_DIR/ota_bin/ota_rtos.bin"
cd "$TOOL_DIR"
./bes_download.exe
