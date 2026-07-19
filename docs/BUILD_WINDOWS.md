# Windows build

## Recommended local path

Run:

```bat
build_plugin_single_click.bat
```

The script downloads the pinned OBS Studio portable package and matching source headers, generates a local `obs.lib` import library, builds the plugin DLL, and creates:

```text
package-programdata\arsonkupik-obs-audio-enhancer
```

The official OBS dependency is stored in:

```text
cmake\OBS_VERSION.txt
```

Using a pinned dependency keeps CI, local packaging, and tagged releases reproducible. Update that file only after compatibility validation.

Then install locally:

```bat
install_plugin_windows.bat
```

## Requirements

- Windows 10/11 x64
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.22+
- Git

## Standalone DSP validation

Configure, build, and run all registered tests through CTest:

```powershell
cmake -S . -B build-dsp -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON -DBUILD_TESTING=ON
cmake --build build-dsp --config Release
ctest --test-dir build-dsp --build-config Release --output-on-failure --timeout 180
```

The registered suite validates multi-level loudness behavior, realtime allocation and dynamics safeguards, selective subsystem rebuilds, settled hard bypass, and preset/bypass transitions. Individual executables remain available under `build-dsp\Release\` for focused debugging.

## Sanitizer validation on Linux or WSL

```bash
cmake -S . -B build-sanitize \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_OBS_PLUGIN=OFF \
  -DBUILD_STANDALONE_TESTS=ON \
  -DBUILD_TESTING=ON \
  -DCMAKE_CXX_FLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-sanitize --parallel
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --test-dir build-sanitize --output-on-failure --timeout 180
```

CI runs the sanitizer suite automatically in addition to Windows and Linux native plugin builds.

## Explicit OBS-version override

The normal build path uses the pinned version. For an intentional compatibility experiment:

```powershell
.\scripts\build-windows-from-obs-release.ps1 -OBSVersion latest
```

Artifacts built against `latest` are experimental and must not be published as official release assets until the pinned dependency is deliberately updated.

## Manual OBS plugin build

```powershell
.\scripts\build-windows.ps1 -Libobs_DIR "C:\path\to\libobs\cmake"
```

Restart OBS after installing, then add the filter from:

```text
Audio Source → Filters → + → ArSonKuPik Smart Enhancer
```

## Channel policy

ArSonKuPik processes the stereo/front-left and front-right pair. Additional channels on a multichannel source are passed through unchanged until a linked multichannel design is implemented and validated.
