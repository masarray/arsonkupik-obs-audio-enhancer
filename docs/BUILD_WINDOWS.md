# Fastest Windows path

For a one-click local build, run from File Explorer or Command Prompt:

```bat
build_plugin_single_click.bat
```

The script downloads OBS Studio portable + OBS source headers, generates a local `obs.lib` import library from `obs.dll`, builds the plugin, and creates:

```text
package-programdatarsonkupik-obs-audio-enhancer```

Then run:

```bat
install_plugin_windows.bat
```

or copy that folder into:

```text
C:\ProgramData\obs-studio\plugins```

---

# Build Windows - ArSonKuPik OBS Audio Enhancer

## Requirements

- Windows 10/11 x64
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.22+
- Git
- OBS/libobs development package or OBS source/build tree where CMake can find `libobsConfig.cmake`

## Option A - Build DSP smoke test only

This does not need OBS/libobs. Use it to confirm the audio engine compiles.

```powershell
cmake -S . -B build-dsp -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-dsp --config Release
.\build-dsp\Release\arsonkupik_dsp_smoke.exe
```

Expected output includes `preset=MasAri` and numeric peak/GR/correlation values.

## Option B - Build OBS plugin DLL

Set `Libobs_DIR` to the folder that contains `libobsConfig.cmake`.

```powershell
.\scripts\build-windows.ps1 -Libobs_DIR "C:\path\to\libobs\cmake"
```

If CMake cannot find libobs, search your OBS build/dependency folder for:

```powershell
Get-ChildItem C:\ -Filter libobsConfig.cmake -Recurse -ErrorAction SilentlyContinue
```

Then pass the parent folder as `-Libobs_DIR`.

## Package for recommended OBS ProgramData layout

After the DLL is built:

```powershell
.\scripts\package-windows-programdata.ps1
```

This creates:

```text
package-programdata\arsonkupik-obs-audio-enhancer\
├─ bin\64bit\arsonkupik-obs-audio-enhancer.dll
└─ data\locale\en-US.ini
```

Copy the `arsonkupik-obs-audio-enhancer` folder to:

```text
C:\ProgramData\obs-studio\plugins\
```

Or run PowerShell as Administrator:

```powershell
.\scripts\package-windows-programdata.ps1 -InstallToProgramData
```

Restart OBS. Add the filter from:

```text
Audio Source → Filters → + → ArSonKuPik Smart Enhancer
```
