<p align="center">
  <a href="https://github.com/masarray/arsonkupik-obs-audio-enhancer">
    <img src="https://raw.githubusercontent.com/masarray/arsonkupik-obs-audio-enhancer/main/assets/images/og-hero.png" alt="ArSonKuPik OBS Audio Enhancer hero banner" width="100%" />
  </a>
</p>

<h1 align="center">ArSonKuPik OBS Audio Enhancer</h1>

<p align="center">
  <a href="LICENSE"><img alt="License: GPL-3.0" src="https://img.shields.io/badge/License-GPLv3-blue.svg"></a>
  <a href="https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/ci.yml/badge.svg"></a>
  <a href="https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/release.yml"><img alt="Release" src="https://github.com/masarray/arsonkupik-obs-audio-enhancer/actions/workflows/release.yml/badge.svg"></a>
  <img alt="Platforms" src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux-informational">
</p>

<p align="center">
  A native OBS Studio audio filter for creators who want media, music, and spoken audio to feel <strong>louder in perception</strong>, <strong>more polished</strong>, and <strong>more alive</strong> without a complicated studio workflow.
</p>

<p align="center">
  <a href="https://github.com/masarray/arsonkupik-obs-audio-enhancer/releases"><strong>Download latest release</strong></a>
  ·
  <a href="docs/BUILD_WINDOWS.md">Build guide</a>
  ·
  <a href="docs/releases/">Release notes</a>
</p>

## What it does

ArSonKuPik is an open-source smart audio enhancement filter for **OBS Studio**. It is built for real listening benefit: when the filter is enabled, users should hear a meaningful improvement in loudness perception, clarity, stereo life, and overall enjoyment.

It is not meant to be a complicated mastering suite. It is a practical OBS-native enhancer for creators, streamers, media playback workflows, podcast-style voices, and users who want a more satisfying sound with simple controls.

## Why ArSonKuPik?

Most OBS audio chains focus on technical correction. ArSonKuPik focuses on how the listener experiences the sound.

- **Louder perception** — designed to feel more present and satisfying without relying on obvious distortion
- **Polished sound** — improves everyday playback with cleaner tonal balance and smarter gain staging
- **Stereo life** — aims for a wider, livelier presentation while protecting center focus
- **Simple workflow** — preset-first controls with live sliders that are easy for normal users to understand
- **Open-source delivery** — GPL-3.0, public releases, Windows installer, Windows ZIP, and Linux archive

## Screenshot

![ArSonKuPik OBS Audio Enhancer UI](assets/images/obs-filter-ui.webp)

## Key features

| Feature | Benefit |
| --- | --- |
| Native OBS filter | Works directly inside OBS as an audio filter |
| Smart enhancement presets | Quick starting points for music, media, podcast, and night listening |
| Perceived loudness benefit | Aims to make Filter ON clearly more useful than raw/bypass audio |
| Stereo integrity approach | Adds life without intentionally making the sound hollow or phasey |
| CPU-conscious DSP | Designed for practical streaming and recording workflows |
| Release automation | Publishes installer, portable ZIP, Linux archive, and public notes |

## Downloads

Every public release is packaged for both regular users and advanced users.

| Platform | Recommended asset | Use case |
| --- | --- | --- |
| Windows | `.exe` installer | Best for most users |
| Windows | portable `.zip` | Manual copy/paste install |
| Linux | `.tar.gz` archive | Manual OBS plugin deployment |

**Latest release:**  
https://github.com/masarray/arsonkupik-obs-audio-enhancer/releases

## Installation

### Windows installer

1. Download the latest `ArSonKuPik-OBS-Audio-Enhancer-Setup-*.exe` from the Releases page.
2. Close OBS Studio.
3. Run the installer.
4. Reopen OBS.
5. Add **ArSonKuPik Smart Enhancer** as an audio filter.

### Windows manual ZIP install

1. Download the latest Windows ZIP asset.
2. Extract the archive.
3. Copy the packaged plugin folder into:
   - `C:\ProgramData\obs-studio\plugins\`
4. Restart OBS.

### Linux manual install

1. Download the latest Linux `.tar.gz` asset.
2. Extract it.
3. Copy the packaged files into your OBS plugin path.
4. Restart OBS.

## Best fit

ArSonKuPik is especially useful for:

- creators who want richer media playback inside OBS
- users who want audio to feel more alive with minimal setup
- podcast or spoken-word workflows that need cleaner perceived presence
- music/media sources that benefit from a more polished listening character
- open-source users who prefer native OBS plugins over browser-only audio enhancement

## Build from source

### Windows

See [docs/BUILD_WINDOWS.md](docs/BUILD_WINDOWS.md).

Quick local build:

```powershell
build_plugin_single_click.bat
```

### Linux

```bash
./scripts/build-linux.sh
```

## Project structure

```text
.github/            GitHub Actions workflows and templates
assets/             README and branding assets
data/               OBS plugin data and locale files
docs/               public docs, release notes, and landing page
docs/site/          GitHub Pages landing site
include/            DSP headers
packaging/windows/  Inno Setup installer script
scripts/            build and packaging helpers
src/                plugin and DSP source
tests/              smoke tests
```

## Documentation

- [Build guide](docs/BUILD_WINDOWS.md)
- [Release automation](docs/RELEASE_AUTOMATION.md)
- [Release notes](docs/releases/)
- [Support](SUPPORT.md)
- [Security policy](SECURITY.md)
- [Contributing](CONTRIBUTING.md)

## License

ArSonKuPik OBS Audio Enhancer is licensed under **GNU General Public License v3.0**. See [LICENSE](LICENSE).
