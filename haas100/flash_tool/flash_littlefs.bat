@echo off
REM Flash ONLY the littlefs filesystem (and pub_otp) to complete a full
REM image after the big littlefs.bin transfer dropped mid-way.
REM
REM Usage: run from the write_flash_tool directory with the board in
REM download mode (hold Download button while powering on).
REM
REM - backs up the normal bes_dld_cfg.yaml
REM - installs the littlefs-only config
REM - runs bes_download.exe
REM - restores the normal config

setlocal
set TOOL_DIR=%~dp0
cd /d "%TOOL_DIR%"

if not exist bes_dld_cfg.yaml goto :nocfg
if not exist bes_dld_cfg_littlefs.yaml goto :nolfs

copy /y bes_dld_cfg.yaml bes_dld_cfg_full.bak >nul
copy /y bes_dld_cfg_littlefs.yaml bes_dld_cfg.yaml >nul

echo Flashing littlefs only...
bes_download.exe

copy /y bes_dld_cfg_full.bak bes_dld_cfg.yaml >nul
del /q bes_dld_cfg_full.bak >nul 2>nul
echo.
echo Restored full flash config.
goto :eof

:nocfg
echo Error: bes_dld_cfg.yaml not found in %TOOL_DIR%
goto :eof

:nolfs
echo Error: bes_dld_cfg_littlefs.yaml not found in %TOOL_DIR%
goto :eof
