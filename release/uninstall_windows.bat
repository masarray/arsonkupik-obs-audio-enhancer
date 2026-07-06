@echo off
setlocal EnableExtensions
title Uninstall ArSonKuPik OBS Audio Enhancer

set "PLUGIN_NAME=arsonkupik-obs-audio-enhancer"
set "PLUGIN_DST=%ProgramData%\obs-studio\plugins\%PLUGIN_NAME%"

echo.
echo ============================================================
echo  ArSonKuPik OBS Audio Enhancer - Windows Uninstaller
echo ============================================================
echo.

net session >nul 2>&1
if not "%errorlevel%"=="0" (
  echo Requesting Administrator permission...
  powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
  exit /b
)

if not exist "%PLUGIN_DST%" (
  echo Plugin folder was not found:
  echo   %PLUGIN_DST%
  echo.
  echo Nothing to uninstall.
  pause
  exit /b 0
)

echo Removing:
echo   %PLUGIN_DST%
echo.
rmdir /s /q "%PLUGIN_DST%"

if exist "%PLUGIN_DST%" (
  echo [ERROR] Could not remove plugin folder. Close OBS Studio and try again.
  pause
  exit /b 1
)

echo [OK] Plugin removed. Restart OBS Studio.
echo.
pause
