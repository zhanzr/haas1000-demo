@echo off
REM Build helper for the HaaS100 helloworld_demo project.
REM Requires: Python 3 + aos-tools + scons (pip install aos-tools),
REM and the arm-none-eabi toolchain at D:\arm-none-eabi-tc (junction to
REM the GNU Arm Embedded Toolchain).

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
echo   App image : out\helloworld_demo@haas100.bin
echo   ELF (gdb): out\helloworld_demo@haas100.elf
echo   Flash set : ..\..\hardware\chip\haas1000\release\release_bin\
endlocal
