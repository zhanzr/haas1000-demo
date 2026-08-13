@echo off
REM Build helper for the HaaS100 coremark_320m benchmark.

setlocal
if not defined PATH set PATH=%PATH%
set PATH=%PATH%;D:\arm-none-eabi-tc\bin

if "%1"=="clean" (
    scons -c --board=haas100
    if exist out rmdir /s /q out
    if exist aos_sdk rmdir /s /q aos_sdk
    goto :eof
)

scons --board=haas100 -j8
if errorlevel 1 (
    echo.
    echo BUILD FAILED
    exit /b 1
)

echo.
echo Build OK.
echo   App image : out\coremark_320m@haas100.bin
endlocal
