# ArSonKuPik Smart Audio Enhancer for OBS

Native OBS audio filter source package based on the ArSonKuPik Chrome extension v0.3.100 engine/preset audit.

## Product target

**ArSonKuPik Smart Enhancer** is an OBS native audio filter intended to be added from:

`Audio Source → Filters → + → ArSonKuPik Smart Enhancer`

The filter is designed around the **MasAri golden reference**: soft/deep bass, pleasant stereo enhancement, silky-air treble, and ear-tickle detail. Other presets are not intended to beat MasAri; they are use-case variations that inherit part of the MasAri auditory DNA.

## User download and install

For normal users, use the GitHub Release ZIP:

```text
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-vX.Y.Z.zip
```

Install flow:

```text
1. Download the ZIP from Releases.
2. Extract it.
3. Close OBS Studio.
4. Double-click install_windows.bat.
5. Restart OBS Studio.
6. Add the filter from Audio Source > Filters > + > ArSonKuPik Smart Enhancer.
```

The release bundle includes:

```text
install_windows.bat
uninstall_windows.bat
README_INSTALL.txt
VERSION.txt
arsonkupik-obs-audio-enhancer/bin/64bit/arsonkupik-obs-audio-enhancer.dll
arsonkupik-obs-audio-enhancer/data/locale/en-US.ini
```

See `docs/USER_INSTALL.md`.

## What was ported from the extension

The extension uses a browser/Web Audio chain:

`input → input gain → smart headroom → safety HPF → PEQ → parallel compressor → color/psychoacoustic enhancer → phase-safe generated side width → limiter → output`

This native OBS version re-implements the same concept in C++:

`input → gain → safety/PEQ biquads → parallel compressor → smart bass/body/air/tickle color → phase-safe stereo magic → soft clipper/limiter → output`

Chrome-specific APIs such as `chrome.tabCapture`, `AudioContext`, `BiquadFilterNode`, `DynamicsCompressorNode`, `WaveShaperNode`, and `HTMLMediaElement.setSinkId` are intentionally not used.

## UI model

OBS UI is intentionally compact:

- Preset
- Bypass
- Smart Protect
- Manual macro tuning
- Enhance
- Smart Bass / Deep Horeg
- Smart Treble / Airy Krenyes
- Vocal Body
- Stereo Magic
- Output Trim

By default, **Manual macro tuning is off**. In that mode, the selected preset uses its frozen factory recipe. Enable Manual macro tuning when you want the sliders to override the preset DNA.

## Included factory presets

- MasAri
- Mastering Global
- Max Enhancer
- SonKuHoreg
- SonKuBattle
- SonKuBalap
- Audiophile
- Punchy Music
- Open Air
- Movie Sub
- Podcast
- Night Listening

## Build smoke test without OBS SDK

This validates the DSP core only:

```bash
./scripts/run-dsp-smoke-test.sh
```

## Build OBS plugin

You need an OBS/libobs development environment or an OBS plugin-template environment where CMake can find `libobs`.

### Linux/macOS-style build

```bash
./scripts/build-linux.sh
```

### Windows PowerShell build

```powershell
./scripts/build-windows.ps1 -Libobs_DIR "C:\path\to\libobs\cmake"
```

The install output is placed under:

`package/`

Manual OBS plugin layout expected by OBS on Windows:

```text
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\bin\64bit\arsonkupik-obs-audio-enhancer.dll
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\data\locale\en-US.ini
```

Depending on your OBS plugin-template packaging setup, you may need to adapt the final folder structure.

## Windows build quick start

See `docs/BUILD_WINDOWS.md` for the full Windows path. Minimal commands:

```powershell
# DSP core smoke test only, no OBS SDK needed
cmake -S . -B build-dsp -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-dsp --config Release
.\build-dsp\Release\arsonkupik_dsp_smoke.exe

# OBS plugin DLL, requires libobsConfig.cmake
.\scripts\build-windows.ps1 -Libobs_DIR "C:\path\to\libobs\cmake"

# Make recommended ProgramData-ready package
.\scripts\package-windows-programdata.ps1
```

Recommended manual install layout on Windows:

```text
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\bin\64bit\arsonkupik-obs-audio-enhancer.dll
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\data\locale\en-US.ini
```

## Engineering notes

The DSP core is in:

- `include/arsonkupik_dsp.hpp`
- `src/arsonkupik_dsp.cpp`
- `src/arsonkupik_presets.cpp`

The OBS wrapper is in:

- `src/arsonkupik_filter.cpp`

The native filter processes OBS float planar audio in-place through `obs_source_info.filter_audio`.

## Windows one-click local build

For Mas Ari's local PC workflow, use:

```bat
build_plugin_single_click.bat
```

This script tries to make the local build as automatic as possible:

1. Finds Visual Studio 2022 C++ tools.
2. Downloads OBS Studio portable + OBS source headers.
3. Generates a local `obs.lib` import library from `obs.dll`.
4. Builds the OBS plugin DLL.
5. Creates a ready-to-copy ProgramData plugin package.

Output folder:

```text
package-programdata\arsonkupik-obs-audio-enhancer
```

To install after building:

```bat
install_plugin_windows.bat
```

## GitHub Actions release build

This repository includes auto-build workflows:

```text
.github/workflows/ci-dsp.yml
.github/workflows/build-windows-plugin.yml
```

- Push to `main` → builds DSP smoke test and Windows OBS plugin artifact.
- Push tag `vX.Y.Z` → creates/updates a GitHub Release with a ready-to-install ZIP.
- Manual workflow run can also publish a release by setting `publish_release = true` and `release_tag = vX.Y.Z`.

See:

```text
docs/GITHUB_AUTOBUILD.md
docs/RELEASE_WORKFLOW.md
```

## Publish public GitHub repo from Windows

Install GitHub CLI, then run:

```bat
publish_public_repo.bat
```

It creates a public GitHub repository and pushes this source package.

## v0.2.4 Build Patch

- Fixes one-click fallback build when raw OBS headers are missing generated `obsconfig.h`.
- Fixes `scripts/package-windows-programdata.ps1` PowerShell array syntax.
- Stops install/package step immediately when compile/link fails.
- `build_plugin_single_click.bat` now cleans previous build output while keeping downloaded OBS dependencies.
- Adds `clean_build_cache.bat`.

## Repo hygiene / avoiding huge pushes

The one-click build downloads OBS into `.deps/` and creates local output folders such as `build-obs-release/`, `package/`, and `package-programdata/`. These folders are intentionally ignored by Git. Run `cleanup_repo_before_push.bat` if a previous commit accidentally staged local build artifacts.
