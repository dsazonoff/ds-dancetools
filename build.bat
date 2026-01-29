@echo off
setlocal enabledelayedexpansion

set INSTALL_DIR=distr
set BIN_NAME=dancetools.exe
set BUILD_DIR_X64=build\release_windows_x64

cmake -S . -B "%BUILD_DIR_X64%" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH=third-party\x64-windows-static ^
  -G "Visual Studio 17 2022" ^
  -A x64

cmake --build "%BUILD_DIR_X64%" --config Release
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
copy "%BUILD_DIR_X64%\bin\%BIN_NAME%" "%INSTALL_DIR%\" >nul
if errorlevel 1 exit /b 1

echo Build finished successfully.
