@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "FOUND_CMAKE="

rem 1) Normal PATH lookup
for /f "delims=" %%C in ('where cmake 2^>nul') do (
  if not defined FOUND_CMAKE set "FOUND_CMAKE=%%C"
)

rem 2) Common standalone CMake install locations
if not defined FOUND_CMAKE if exist "%ProgramFiles%\CMake\bin\cmake.exe" set "FOUND_CMAKE=%ProgramFiles%\CMake\bin\cmake.exe"
if not defined FOUND_CMAKE if exist "%ProgramFiles(x86)%\CMake\bin\cmake.exe" set "FOUND_CMAKE=%ProgramFiles(x86)%\CMake\bin\cmake.exe"

rem 3) Visual Studio bundled CMake, usually not exposed in PATH
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not defined FOUND_CMAKE if exist "%VSWHERE%" (
  for /f "usebackq delims=" %%C in (`"%VSWHERE%" -latest -products * -find "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"`) do (
    if not defined FOUND_CMAKE set "FOUND_CMAKE=%%C"
  )
)

rem 4) Manual scan for VS 2022 / future VS versions such as 2026 previews
if not defined FOUND_CMAKE (
  for /f "delims=" %%C in ('dir /s /b "%ProgramFiles%\Microsoft Visual Studio\*\*\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" 2^>nul') do (
    if not defined FOUND_CMAKE set "FOUND_CMAKE=%%C"
  )
)
if not defined FOUND_CMAKE (
  for /f "delims=" %%C in ('dir /s /b "%ProgramFiles(x86)%\Microsoft Visual Studio\*\*\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" 2^>nul') do (
    if not defined FOUND_CMAKE set "FOUND_CMAKE=%%C"
  )
)

if not defined FOUND_CMAKE (
  echo [ERROR] CMake was not found.
  echo.
  echo This can happen even if Visual Studio / VS Code can build projects,
  echo because their bundled CMake is not always added to Windows PATH.
  echo.
  echo Fix options:
  echo   1. Open Visual Studio Installer ^> Modify ^> Individual components ^> C++ CMake tools for Windows
  echo   2. Install standalone CMake and tick "Add CMake to PATH"
  echo   3. Or run from a Developer PowerShell where cmake is visible
  echo.
  exit /b 1
)

for %%D in ("%FOUND_CMAKE%") do set "CMAKE_DIR=%%~dpD"
endlocal & set "CMAKE_EXE=%FOUND_CMAKE%" & set "PATH=%CMAKE_DIR%;%PATH%" & exit /b 0
