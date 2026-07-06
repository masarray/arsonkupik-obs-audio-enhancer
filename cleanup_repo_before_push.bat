@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title Cleanup repo before GitHub push

echo ============================================================
echo  Cleanup build/dependency artifacts before GitHub push
echo ============================================================
echo.

if not exist .git (
  echo [INFO] Git repository is not initialized yet. Nothing to untrack.
) else (
  echo Removing generated folders from Git index if they were staged/tracked...
  git rm -r --cached .deps build build-dsp build-obs-release package package-programdata artifacts out 2>nul
  git rm -r --cached *.zip *.dll *.exe *.pdb *.lib *.exp *.ilk 2>nul
)

echo.
echo Local folders are NOT deleted. They are only ignored/untracked from Git.
echo Updated .gitignore will prevent future accidental large push.
echo.
echo Recommended next commands:
echo   git add .gitignore cleanup_repo_before_push.bat
if exist .git echo   git status --short --ignored
if exist .git echo   git commit -m "Ignore local build artifacts and dependency cache"
echo.
pause
