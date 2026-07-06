@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title Check Git push size candidates

echo ============================================================
echo  Files that would make push huge
 echo ============================================================
echo.

if not exist .git (
  echo [ERROR] This folder is not a Git repository yet.
  pause
  exit /b 1
)

echo Top large files currently tracked by Git:
powershell -NoProfile -ExecutionPolicy Bypass -Command "git ls-files | ForEach-Object { if (Test-Path $_) { $i=Get-Item $_; [PSCustomObject]@{SizeMB=[math]::Round($i.Length/1MB,2); File=$_} } } | Sort-Object SizeMB -Descending | Select-Object -First 30 | Format-Table -AutoSize"

echo.
echo Ignored folders/files detected locally:
git status --ignored --short

echo.
pause
