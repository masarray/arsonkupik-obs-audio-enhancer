@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title Install ArSonKuPik OBS Audio Enhancer

set "PLUGIN_NAME=arsonkupik-obs-audio-enhancer"
set "PLUGIN_SRC=%~dp0%PLUGIN_NAME%"
set "OBS_PLUGIN_DIR=%ProgramData%\obs-studio\plugins"
set "PLUGIN_DST=%OBS_PLUGIN_DIR%\%PLUGIN_NAME%"

echo.
echo ============================================================
echo  ArSonKuPik OBS Audio Enhancer - Windows Installer
echo ============================================================
echo.

if not exist "%PLUGIN_SRC%\bin\64bit\arsonkupik-obs-audio-enhancer.dll" (
  echo [ERROR] Plugin files were not found next to this installer.
  echo Expected:
  echo   %PLUGIN_SRC%\bin\64bit\arsonkupik-obs-audio-enhancer.dll
  echo.
  echo Please extract the release ZIP first, then run this installer again.
  echo.
  pause
  exit /b 1
)

net session >nul 2>&1
if not "%errorlevel%"=="0" (
  echo Requesting Administrator permission...
  powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
  exit /b
)

echo Installing to:
echo   %PLUGIN_DST%
echo.

if not exist "%OBS_PLUGIN_DIR%" mkdir "%OBS_PLUGIN_DIR%"
if exist "%PLUGIN_DST%" (
  echo Removing previous installation...
  rmdir /s /q "%PLUGIN_DST%"
)

xcopy "%PLUGIN_SRC%" "%PLUGIN_DST%" /E /I /Y >nul
if errorlevel 1 (
  echo [ERROR] Install copy failed.
  pause
  exit /b 1
)

echo.
echo [OK] ArSonKuPik OBS Audio Enhancer installed.
echo Restart OBS Studio, then add it from:
echo   Audio Source ^> Filters ^> + ^> ArSonKuPik Smart Enhancer
echo.
pause
