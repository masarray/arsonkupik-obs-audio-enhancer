@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title Install ArSonKuPik OBS Plugin

echo Installing ArSonKuPik OBS plugin to ProgramData...
echo This requires Administrator permission.
echo.

if not exist "%~dp0package-programdata\arsonkupik-obs-audio-enhancer\bin\64bit\arsonkupik-obs-audio-enhancer.dll" (
  echo [ERROR] Built plugin package was not found.
  echo Run build_plugin_single_click.bat first.
  pause
  exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process PowerShell -Verb RunAs -ArgumentList '-NoProfile -ExecutionPolicy Bypass -File ''%~dp0scripts\package-windows-programdata.ps1'' -InstallToProgramData'"
echo.
echo If Windows asked for Administrator permission, installation is running in a new window.
echo Restart OBS after installation finishes.
pause
