# Windows build

## Recommended local path

Run:

```bat
build_plugin_single_click.bat
```

The script downloads OBS Studio portable + OBS source headers, generates a local `obs.lib` import library, builds the plugin DLL, and creates:

```text
package-programdata\arsonkupik-obs-audio-enhancer
```

Then install locally:

```bat
install_plugin_windows.bat
```

## Requirements

- Windows 10/11 x64
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.22+
- Git

## DSP smoke test only

```powershell
cmake -S . -B build-dsp -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-dsp --config Release
.\build-dsp\Release\arsonkupik_dsp_smoke.exe
```

## Manual OBS plugin build

```powershell
.\scripts\build-windows.ps1 -Libobs_DIR "C:\path\to\libobs\cmake"
```

Restart OBS after installing, then add the filter from:

```text
Audio Source → Filters → + → ArSonKuPik Smart Enhancer
```
