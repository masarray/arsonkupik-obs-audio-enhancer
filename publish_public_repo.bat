@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title Publish ArSonKuPik OBS Plugin to GitHub

echo ============================================================
echo  Publish repo to GitHub public repository
echo ============================================================
echo.

where git >nul 2>nul
if errorlevel 1 (
  echo [ERROR] Git was not found in PATH.
  pause
  exit /b 1
)

where gh >nul 2>nul
if errorlevel 1 (
  echo [ERROR] GitHub CLI was not found in PATH.
  echo Install GitHub CLI, then run: gh auth login
  pause
  exit /b 1
)

gh auth status >nul 2>nul
if errorlevel 1 (
  echo GitHub CLI is not logged in. Starting login...
  gh auth login
  if errorlevel 1 (
    echo [ERROR] GitHub login failed.
    pause
    exit /b 1
  )
)

set "DEFAULT_REPO=arsonkupik-obs-audio-enhancer"
set /p REPO_NAME="Repository name [%DEFAULT_REPO%]: "
if "%REPO_NAME%"=="" set "REPO_NAME=%DEFAULT_REPO%"

if not exist .git (
  git init
  git branch -M main
)

echo.
echo [INFO] Ensuring local build/dependency cache is not staged...
git rm -r --cached .deps build build-dsp build-obs-release package package-programdata artifacts out 2>nul
git rm -r --cached *.zip *.dll *.exe *.pdb *.lib *.exp *.ilk 2>nul

git add .
git commit -m "Initial ArSonKuPik OBS native audio enhancer" || echo No new commit needed.

echo.
echo Creating/pushing public repo: %REPO_NAME%
gh repo create "%REPO_NAME%" --public --source=. --remote=origin --push
if errorlevel 1 (
  echo.
  echo [WARN] Repo creation may have failed or the repo may already exist.
  echo If the repo already exists, set remote manually then push:
  echo   git remote add origin https://github.com/YOUR_USER/%REPO_NAME%.git
  echo   git push -u origin main
  pause
  exit /b 1
)

echo.
echo DONE. GitHub Actions will start automatically after push.
gh repo view --web
pause
