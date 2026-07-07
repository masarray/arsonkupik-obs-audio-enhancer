# ArSonKuPik OBS Audio Enhancer

[![License: GPL-3.0](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![CI](https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/ci.yml/badge.svg)](https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/ci.yml)
[![Release](https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/release.yml/badge.svg)](https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/release.yml)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-informational)](#downloads)

Professional smart audio enhancement filter for **OBS Studio**. ArSonKuPik is designed to make everyday playback, music, media, and spoken-word content sound **louder in perception, more polished, more alive, and more enjoyable** while keeping real-time OBS performance practical.

> Goal: when the filter is ON, users should hear a meaningful benefit in loudness perception, clarity, stereo life, and listening enjoyment.

## Screenshot

![ArSonKuPik OBS Audio Enhancer UI](assets/images/obs-filter-ui.webp)

## Highlights

- Smart loudness benefit tuned for real-world listener perception
- Musical presets for music, media, podcast, and night listening
- OBS-native audio filter workflow
- Stereo integrity-first enhancement designed to preserve center focus
- CPU-conscious DSP direction for live streaming
- Automated public releases with installer, portable ZIP, Linux package, and release notes

## Downloads

Use the latest GitHub Release:

- **Windows installer `.exe`** — recommended for most users
- **Windows portable `.zip`** — manual copy/paste install
- **Linux `.tar.gz`** — manual Linux package artifact

GitHub Releases: https://github.com/masarray/arsonkupik-obs-audio-enhancer/releases

## Install

### Windows installer

1. Close OBS Studio.
2. Download `ArSonKuPik-OBS-Audio-Enhancer-Setup-vX.Y.Z.exe`.
3. Run the installer as Administrator.
4. Restart OBS Studio.
5. Add the filter from **Audio Source → Filters → + → ArSonKuPik Smart Enhancer**.

The installer uses the standard OBS ProgramData plugin location:

```text
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer
```

### Windows portable ZIP

1. Close OBS Studio.
2. Extract the ZIP.
3. Copy the `arsonkupik-obs-audio-enhancer` folder into:

```text
C:\ProgramData\obs-studio\plugins\
```

4. Restart OBS.

### Linux

Extract the Linux `.tar.gz` release asset and copy the package contents into the appropriate OBS plugin path for your distribution/package style.

## Local Windows build

Only two root `.bat` files are kept intentionally:

```text
build_plugin_single_click.bat
install_plugin_windows.bat
```

`build_plugin_single_click.bat` builds the OBS plugin and creates a ProgramData-ready package.
`install_plugin_windows.bat` installs that local package into the standard OBS ProgramData plugin folder.

The build helper uses `scripts/find-cmake.bat` internally to find CMake, including Visual Studio bundled CMake.

## Release automation

Create a tag such as `v0.4.4` to trigger the release workflow. The workflow builds and publishes:

- Windows installer `.exe`
- Windows portable `.zip`
- Linux `.tar.gz`
- public, user-facing release notes

See [docs/RELEASE_AUTOMATION.md](docs/RELEASE_AUTOMATION.md).

## Repository structure

```text
.github/            GitHub Actions workflows and templates
assets/             README and branding assets
data/               OBS plugin data and locale files
docs/               public docs, release notes, and landing page
include/            DSP headers
packaging/windows/  Inno Setup installer script
scripts/            build/package scripts
src/                plugin + DSP source
tests/              smoke tests
```

## License

Licensed under the **GNU General Public License v3.0**. See [LICENSE](LICENSE).

ArSonKuPik is an independent open-source project and is not affiliated with or endorsed by OBS Project. OBS and OBS Studio are names/trademarks of their respective owners.
