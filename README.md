# ArSonKuPik OBS Audio Enhancer

<p align="center">
  <strong>Smart audio enhancement filter for OBS Studio: MasAri bass, silky treble, stereo magic, and one-click streaming presets.</strong>
</p>

<p align="center">
  <a href="https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/build-windows-plugin.yml"><img alt="Windows build" src="https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/build-windows-plugin.yml/badge.svg"></a>
  <a href="https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/ci-dsp.yml"><img alt="DSP CI" src="https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/ci-dsp.yml/badge.svg"></a>
  <a href="https://github.com/masarray/arsonkupik-obs-audio-enhancer/releases"><img alt="GitHub release" src="https://img.shields.io/github/v/release/masarray/arsonkupik-obs-audio-enhancer?include_prereleases&label=release"></a>
  <a href="LICENSE"><img alt="License" src="https://img.shields.io/badge/license-Apache--2.0-blue.svg"></a>
  <img alt="Platform" src="https://img.shields.io/badge/platform-Windows%20x64-lightgrey.svg">
  <img alt="OBS Studio" src="https://img.shields.io/badge/OBS%20Studio-native%20audio%20filter-302E31.svg">
</p>

**ArSonKuPik OBS Audio Enhancer** is a native OBS Studio audio filter for creators, streamers, YouTubers, live sellers, karaoke hosts, podcasters, and music reviewers who want a simple way to make audio sound more alive without building a complex chain of EQ, compressor, stereo widener, exciter, and limiter filters.

It is designed around the **MasAri sonic reference**: soft deep bass, pleasant stereo enhancement, silky-air treble, vocal/body clarity, and ear-tickle detail that feels musical instead of harsh.

> Add it in OBS from: **Audio Source → Filters → + → ArSonKuPik Smart Enhancer**

---

## Why this plugin exists

Most OBS audio workflows require multiple technical filters and careful gain staging. ArSonKuPik aims to make the process easier:

- **One plugin** for smart tone, body, width, compression, and safety limiting.
- **One-click presets** for music, voice, karaoke, loudness, night listening, and clean audiophile use.
- **MasAri reference sound** for bass warmth, stereo width, and silky treble.
- **Simple controls** for non-technical users: Smart Bass, Smart Treble, Vocal Body, Stereo Magic, and Smart Protect.
- **Native OBS filter** implementation, not a browser wrapper.

---

## Download and install

Download the latest Windows package from:

**Releases:** https://github.com/masarray/arsonkupik-obs-audio-enhancer/releases

Get the ZIP named like:

```text
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-vX.Y.Z.zip
```

Install:

```text
1. Close OBS Studio.
2. Extract the ZIP.
3. Double-click install_windows.bat.
4. Approve Administrator permission if Windows asks.
5. Restart OBS Studio.
6. Add the filter from Audio Source > Filters > + > ArSonKuPik Smart Enhancer.
```

Recommended first setting:

```text
Preset: MasAri
Manual macro tuning: OFF
Smart Protect: ON
Bypass: OFF
```

Manual install path:

```text
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\bin\64bit\arsonkupik-obs-audio-enhancer.dll
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\data\locale\en-US.ini
```

See also: [`docs/USER_INSTALL.md`](docs/USER_INSTALL.md)

---

## Core features

### Smart presets

Included factory presets:

| Preset | Best for | Character |
|---|---|---|
| **MasAri** | Default music and streaming | Soft bass, stereo magic, silky air, ear tickles |
| **Mastering Global** | General program audio | Balanced enhancer for everyday use |
| **Max Enhancer** | Strong enhancement | More color, body, width, and loudness |
| **SonKuHoreg** | Deep bass / horeg feel | Bigger low-end pressure with protection |
| **SonKuBattle** | Loudness and projection | More aggressive, denser, battle-style sound |
| **SonKuBalap** | Fast energetic music | Punch, projection, and forward energy |
| **Audiophile** | Clean listening | Subtle enhancement with minimal coloration |
| **Punchy Music** | Beat-driven music | Punch, rhythm, and body |
| **Open Air** | Wide and bright music | Airy, open, spacious presentation |
| **Movie Sub** | Movie/game audio | Deeper cinematic low-end |
| **Podcast** | Voice and talking content | Clearer voice, body, and controlled brightness |
| **Night Listening** | Long listening / low volume | Smooth, less harsh, comfortable sound |

### Simple OBS controls

The UI is intentionally compact:

- **Preset**
- **Bypass** with smooth internal A/B switching
- **Smart Protect**
- **Manual macro tuning**
- **Enhance**
- **Smart Bass / Deep Horeg**
- **Smart Treble / Airy Krenyes**
- **Vocal Body**
- **Stereo Magic**
- **Output Trim**

By default, **Manual macro tuning is off**. In that mode, the selected preset uses its factory recipe. Enable Manual macro tuning only when you want to override the preset with your own macro slider values.

---

## Audio engine

The original ArSonKuPik extension used a browser/Web Audio concept:

```text
input → input gain → smart headroom → safety HPF → PEQ → parallel compressor → color/psychoacoustic enhancer → phase-safe generated side width → limiter → output
```

This native OBS version re-implements the concept in C++:

```text
input → gain → safety/PEQ biquads → parallel compressor → smart bass/body/air/tickle color → phase-safe stereo magic → soft clipper/limiter → output
```

Chrome-specific APIs such as `chrome.tabCapture`, `AudioContext`, `BiquadFilterNode`, `DynamicsCompressorNode`, `WaveShaperNode`, and `HTMLMediaElement.setSinkId` are intentionally not used.

Key DSP goals:

- Bass should feel **deep and soft**, not boomy.
- Treble should feel **airy, silky, and krenyes**, not sharp.
- Stereo enhancement should feel wide while keeping the vocal/body center stable.
- Smart Protect should reduce clipping and unsafe peaks during live streaming.

---

## Build from source

### Windows one-click local build

For local development on Windows:

```bat
build_plugin_single_click.bat
```

The script attempts to:

1. Find Visual Studio C++ tools.
2. Download OBS Studio portable and OBS source headers.
3. Generate a local `obs.lib` import library from `obs.dll`.
4. Build the OBS plugin DLL.
5. Create a ProgramData-ready plugin package.

Output folder:

```text
package-programdata\arsonkupik-obs-audio-enhancer
```

Install locally:

```bat
install_plugin_windows.bat
```

### DSP smoke test only

This validates the DSP core without OBS SDK:

```bash
./scripts/run-dsp-smoke-test.sh
```

Or with CMake:

```powershell
cmake -S . -B build-dsp -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-dsp --config Release
.\build-dsp\Release\arsonkupik_dsp_smoke.exe
```

### Manual OBS plugin build

```powershell
.\scripts\build-windows.ps1 -Libobs_DIR "C:\path\to\libobs\cmake"
```

Developer docs:

- [`docs/BUILD_WINDOWS.md`](docs/BUILD_WINDOWS.md)
- [`docs/BUILD_TROUBLESHOOTING.md`](docs/BUILD_TROUBLESHOOTING.md)
- [`docs/RELEASE_WORKFLOW.md`](docs/RELEASE_WORKFLOW.md)
- [`docs/GITHUB_AUTOBUILD.md`](docs/GITHUB_AUTOBUILD.md)

---

## GitHub Actions release workflow

This repository includes automatic Windows package builds:

```text
.github/workflows/ci-dsp.yml
.github/workflows/build-windows-plugin.yml
```

Release flow:

```powershell
git tag v0.2.6
git push origin v0.2.6
```

The workflow builds the native OBS plugin, creates a ready-to-install ZIP, creates a `.sha256` checksum, and publishes a GitHub Release.

---

## Project structure

```text
include/arsonkupik_dsp.hpp        DSP API and engine data model
src/arsonkupik_dsp.cpp            Native DSP implementation
src/arsonkupik_presets.cpp        Factory preset recipes
src/arsonkupik_filter.cpp         OBS filter wrapper
data/locale/en-US.ini             OBS UI labels
release/                          Installer, uninstaller, user install notes
docs/                             Build, release, SEO, and user docs
tests/dsp_smoke_test.cpp          Standalone DSP smoke test
```

The native filter processes OBS float planar audio in-place through the OBS audio filter callback.

---

## SEO keywords

OBS audio enhancer, OBS audio filter, OBS Studio plugin, OBS plugin Windows, OBS music enhancer, OBS voice enhancer, OBS mastering plugin, OBS limiter, OBS compressor, OBS stereo enhancer, OBS bass enhancer, OBS treble enhancer, streaming audio enhancer, YouTube audio enhancer, karaoke OBS plugin, podcast OBS plugin, native OBS plugin, C++ OBS plugin, ArSonKuPik, MasAri preset, smart audio enhancement.

---

## Repository topics to add on GitHub

For better discovery, set these repository topics in **GitHub → About → Settings gear → Topics**:

```text
obs obs-studio obs-plugin obs-audio-filter audio-enhancer audio-plugin streaming streaming-tools podcast karaoke music-production cplusplus cpp cmake windows native-plugin audio-dsp limiter compressor stereo-enhancer bass-enhancer open-source apache-2-0
```

Recommended GitHub repository description:

```text
Native OBS Studio audio enhancer plugin for streaming, music, podcast, karaoke, bass, treble, stereo width, compressor and limiter presets.
```

---

## License

This project is licensed under the **Apache License 2.0**. See [`LICENSE`](LICENSE) and [`NOTICE`](NOTICE).

Important compatibility note: this repository contains original ArSonKuPik source code and build scripts under Apache-2.0. The plugin is designed to build against OBS Studio / libobs APIs, so binary distribution should also respect the applicable OBS Studio and third-party dependency license terms.

---

## Contributing

Contributions are welcome. Please read [`CONTRIBUTING.md`](CONTRIBUTING.md), [`SECURITY.md`](SECURITY.md), and [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md) before opening issues or pull requests.

Useful contribution areas:

- Preset tuning and listening tests
- DSP optimization
- OBS compatibility testing
- Windows installer packaging
- Documentation and translations
- UX naming for creator-friendly controls

---

## Disclaimer

ArSonKuPik is an independent open-source project and is not affiliated with or endorsed by OBS Project. OBS and OBS Studio are names/trademarks of their respective owners.
