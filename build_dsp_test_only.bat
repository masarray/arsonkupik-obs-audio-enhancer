@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title ArSonKuPik DSP Smoke Test Build

echo ============================================================
echo  ArSonKuPik DSP Smoke Test - No OBS SDK Required
echo ============================================================
echo.

where cmake >nul 2>nul
if errorlevel 1 (
  echo [ERROR] CMake was not found in PATH.
  pause
  exit /b 1
)

cmake -S . -B build-dsp -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
if errorlevel 1 goto failed
cmake --build build-dsp --config Release
if errorlevel 1 goto failed

if exist build-dsp\Release\arsonkupik_dsp_smoke.exe (
  build-dsp\Release\arsonkupik_dsp_smoke.exe
) else if exist build-dsp\arsonkupik_dsp_smoke.exe (
  build-dsp\arsonkupik_dsp_smoke.exe
) else (
  echo [WARN] Smoke test executable was built but not found in the expected path.
)

echo.
echo DONE.
pause
exit /b 0

:failed
echo.
echo [FAILED] DSP build failed.
pause
exit /b 1
