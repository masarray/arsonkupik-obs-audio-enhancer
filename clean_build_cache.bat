@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title ArSonKuPik OBS Plugin - Clean Build Cache

echo Cleaning ArSonKuPik OBS build cache...
echo.
if exist build-obs-release rmdir /s /q build-obs-release
if exist build rmdir /s /q build
if exist build-dsp rmdir /s /q build-dsp
if exist package rmdir /s /q package
if exist package-programdata rmdir /s /q package-programdata

echo Done. Downloaded OBS dependencies in .deps are kept.
echo Run build_plugin_single_click.bat again.
echo.
pause
