@echo off
REM Flash the FULL image set: boot1 + boot_info + boot2a + RTOS
REM + littlefs filesystem + boot1_sec + factory + pub_otp.
REM
REM The default bes_dld_cfg.yaml only flashes the boot-critical images
REM + RTOS (fast, no 4.9 MB littlefs). Use this to ALSO program the
REM filesystem and the optional regions.
REM
REM Usage: run from the write_flash_tool directory with the board in
REM download mode (hold Download button while powering on).
REM
REM - backs up the minimal bes_dld_cfg.yaml
REM - installs bes_dld_cfg_full.yaml
REM - runs bes_download.exe
REM - restores the minimal config

setlocal
set TOOL_DIR=%~dp0
cd /d "%TOOL_DIR%"

if not exist bes_dld_cfg.yaml goto :nocfg
if not exist bes_dld_cfg_full.yaml goto :nofull

copy /y bes_dld_cfg.yaml bes_dld_cfg_min.bak >nul
copy /y bes_dld_cfg_full.yaml bes_dld_cfg.yaml >nul

echo Flashing full image set (incl. littlefs)... this can take ~1 minute.
bes_download.exe

copy /y bes_dld_cfg_min.bak bes_dld_cfg.yaml >nul
del /q bes_dld_cfg_min.bak >nul 2>nul
echo.
echo Restored minimal flash config.
goto :eof

:nocfg
echo Error: bes_dld_cfg.yaml not found in %TOOL_DIR%
goto :eof

:nofull
echo Error: bes_dld_cfg_full.yaml not found in %TOOL_DIR%
goto :eof
