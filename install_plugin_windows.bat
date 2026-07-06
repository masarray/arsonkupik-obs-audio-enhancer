@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title Install ArSonKuPik OBS Audio Enhancer

set "PLUGIN_NAME=arsonkupik-obs-audio-enhancer"
set "PLUGIN_SRC=%~dp0package-programdata\%PLUGIN_NAME%"
set "PLUGIN_DLL=%PLUGIN_SRC%\bin\64bit\%PLUGIN_NAME%.dll"
set "OBS_PLUGIN_ROOT=%ProgramData%\obs-studio\plugins"
set "PLUGIN_DST=%OBS_PLUGIN_ROOT%\%PLUGIN_NAME%"

echo.
echo ============================================================
echo  ArSonKuPik OBS Audio Enhancer - Local Install
echo ============================================================
echo.

if not exist "%PLUGIN_DLL%" (
  echo [ERROR] Built plugin package was not found.
  echo Expected DLL:
  echo   %PLUGIN_DLL%
  echo.
  echo Run build_plugin_single_click.bat first.
  echo.
  pause
  exit /b 1
)

net session >nul 2>&1
if not "%errorlevel%"=="0" (
  echo [ERROR] Administrator permission is required.
  echo.
  echo Close this window, then right-click install_plugin_windows.bat and choose:
  echo   Run as administrator
  echo.
  pause
  exit /b 1
)

echo Source:
echo   %PLUGIN_SRC%
echo.
echo Target:
echo   %PLUGIN_DST%
echo.

if not exist "%OBS_PLUGIN_ROOT%" mkdir "%OBS_PLUGIN_ROOT%"
if exist "%PLUGIN_DST%" rmdir /s /q "%PLUGIN_DST%"

xcopy "%PLUGIN_SRC%" "%PLUGIN_DST%" /E /I /Y
if errorlevel 1 (
  echo.
  echo [ERROR] Copy failed. Close OBS Studio and try again.
  echo.
  pause
  exit /b 1
)

if not exist "%PLUGIN_DST%\bin\64bit\%PLUGIN_NAME%.dll" (
  echo.
  echo [ERROR] Install verification failed.
  echo DLL was not found at:
  echo   %PLUGIN_DST%\bin\64bit\%PLUGIN_NAME%.dll
  echo.
  pause
  exit /b 1
)

echo.
echo [OK] Installed successfully.
echo.
echo Installed DLL:
echo   %PLUGIN_DST%\bin\64bit\%PLUGIN_NAME%.dll
echo.
echo Restart OBS Studio, then add:
echo   Audio Source ^> Filters ^> + ^> ArSonKuPik Smart Enhancer
echo.
pause
