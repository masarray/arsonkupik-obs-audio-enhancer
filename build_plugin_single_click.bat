@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title ArSonKuPik OBS Plugin - One Click Build

echo ============================================================
echo  ArSonKuPik OBS Audio Enhancer - One Click Windows Build
echo ============================================================
echo.

where cmake >nul 2>nul
if errorlevel 1 (
  echo [ERROR] CMake was not found in PATH.
  echo Install CMake from https://cmake.org/download/ and enable Add CMake to PATH.
  pause
  exit /b 1
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo [ERROR] Visual Studio 2022 was not found.
  echo Install Visual Studio 2022 with Desktop development with C++ workload.
  pause
  exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL (
  echo [ERROR] Visual Studio C++ toolchain was not found.
  echo Open Visual Studio Installer and add Desktop development with C++.
  pause
  exit /b 1
)

set "VSDEVCMD=%VSINSTALL%\Common7\Tools\VsDevCmd.bat"
if not exist "%VSDEVCMD%" (
  echo [ERROR] VsDevCmd.bat was not found: %VSDEVCMD%
  pause
  exit /b 1
)

echo Using Visual Studio: %VSINSTALL%
echo.
echo This script will download OBS Studio portable + source headers,
echo generate a local obs.lib import library, build the plugin DLL,
echo and create a ProgramData-ready plugin folder.
echo.

echo Starting build...
call "%VSDEVCMD%" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 (
  echo [ERROR] Failed to initialize Visual Studio x64 developer environment.
  pause
  exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\build-windows-from-obs-release.ps1"
if errorlevel 1 (
  echo.
  echo [FAILED] Build failed. Please check the error above.
  pause
  exit /b 1
)

echo.
echo ============================================================
echo  BUILD SUCCESS
echo ============================================================
echo Ready-to-copy OBS plugin folder:
echo   %~dp0package-programdata\arsonkupik-obs-audio-enhancer
echo.
echo To install manually, copy that folder to:
echo   C:\ProgramData\obs-studio\plugins\
echo.
echo Or run install_plugin_windows.bat as Administrator.
echo.
pause
