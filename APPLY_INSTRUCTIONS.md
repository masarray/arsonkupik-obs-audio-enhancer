# Apply ArSonKuPik OBS Audio Enhancer v0.4.12 patch

This patch keeps the v0.4.11 VST-style macro controls but recalibrates gain staging so Filter ON is not excessively louder than bypass.

## Apply

```powershell
cd C:\Git\arsonkupik-obs-audio-enhancer

$zip  = "$env:USERPROFILE\Downloads\arsonkupik-v0_4_12-calibrated-gain-staging-patch-files.zip"
$temp = "$env:TEMP\arsonkupik-v0412-gain"

Remove-Item $temp -Recurse -Force -ErrorAction SilentlyContinue
Expand-Archive $zip -DestinationPath $temp -Force

Copy-Item "$temp\src\arsonkupik_dsp.cpp" "src\arsonkupik_dsp.cpp" -Force
Copy-Item "$temp\src\arsonkupik_filter.cpp" "src\arsonkupik_filter.cpp" -Force
Copy-Item "$temp\tests\dsp_smoke_test.cpp" "tests\dsp_smoke_test.cpp" -Force
Copy-Item "$temp\CMakeLists.txt" "CMakeLists.txt" -Force
Copy-Item "$temp\CHANGELOG.md" "CHANGELOG.md" -Force
Copy-Item "$temp\docs\releases\v0.4.12.md" "docs\releases\v0.4.12.md" -Force
Copy-Item "$temp\docs\releases\latest.md" "docs\releases\latest.md" -Force
```

## Build and install

```powershell
build_plugin_single_click.bat
install_plugin_windows.bat
```

## OBS listening test

- Compare bypass versus Filter ON.
- Target: ON should feel more polished and slightly more exciting, around +3 to +4 dB, not a huge volume jump.
- Sweep Smart Bass, Smart Treble, Vocal Body, and Stereo Magic. The effect should still be obvious.
- Confirm there is no crackle, no heavy pumping, and no sudden limiter crush.

## Commit and release

```powershell
git status
git add src/arsonkupik_dsp.cpp src/arsonkupik_filter.cpp tests/dsp_smoke_test.cpp CMakeLists.txt CHANGELOG.md docs/releases/v0.4.12.md docs/releases/latest.md
git commit -m "Calibrate gain staging after VST-style macro port"
git push origin main

git tag -a v0.4.12 -m "ArSonKuPik OBS Audio Enhancer v0.4.12"
git push origin v0.4.12
```
