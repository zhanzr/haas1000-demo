#!/usr/bin/env sh
# Flash ONLY the littlefs filesystem (and pub_otp) to complete a full
# image after the big littlefs.bin transfer dropped mid-way.
# Usage: run from the write_flash_tool directory with the board in
# download mode (hold Download button while powering on).
set -e
cd "$(dirname "$0")"
[ -f bes_dld_cfg.yaml ] || { echo "error: bes_dld_cfg.yaml not found in $(pwd)"; exit 1; }
[ -f bes_dld_cfg_littlefs.yaml ] || { echo "error: bes_dld_cfg_littlefs.yaml not found"; exit 1; }
cp bes_dld_cfg.yaml bes_dld_cfg_full.bak
cp bes_dld_cfg_littlefs.yaml bes_dld_cfg.yaml
echo "Flashing littlefs only..."
./bes_download.exe
cp bes_dld_cfg_full.bak bes_dld_cfg.yaml
rm -f bes_dld_cfg_full.bak
echo "Restored full flash config."
