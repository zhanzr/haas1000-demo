#!/usr/bin/env sh
# Flash the FULL image set: boot1 + boot_info + boot2a + RTOS + littlefs
# + boot1_sec + factory + pub_otp. The default bes_dld_cfg.yaml only
# flashes the boot-critical images + RTOS; use this to also program the
# filesystem and optional regions.
# Usage: run from the write_flash_tool directory with the board in
# download mode (hold Download button while powering on).
set -e
cd "$(dirname "$0")"
[ -f bes_dld_cfg.yaml ] || { echo "error: bes_dld_cfg.yaml not found in $(pwd)"; exit 1; }
[ -f bes_dld_cfg_full.yaml ] || { echo "error: bes_dld_cfg_full.yaml not found"; exit 1; }
cp bes_dld_cfg.yaml bes_dld_cfg_min.bak
cp bes_dld_cfg_full.yaml bes_dld_cfg.yaml
echo "Flashing full image set (incl. littlefs)... this can take ~1 minute."
./bes_download.exe
cp bes_dld_cfg_min.bak bes_dld_cfg.yaml
rm -f bes_dld_cfg_min.bak
echo "Restored minimal flash config."
