# Contributing to ArSonKuPik OBS Audio Enhancer

Thank you for helping improve ArSonKuPik OBS Audio Enhancer.

This project focuses on a simple, creator-friendly OBS audio enhancement workflow: smart presets, bass warmth, silky treble, stereo magic, and safe live-streaming output.

## Good first contribution areas

- Test the plugin on different OBS Studio versions.
- Improve preset translation and listening notes.
- Report audio artifacts, clipping, phase issues, or harsh treble.
- Improve Windows packaging and installation UX.
- Improve documentation and translations.
- Optimize DSP performance.

## Development setup

For Windows local build:

```bat
build_plugin_single_click.bat
```

For DSP-only test:

```powershell
cmake -S . -B build-dsp -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-dsp --config Release
.\build-dsp\Release\arsonkupik_dsp_smoke.exe
```

## Pull request checklist

Before opening a pull request:

- Run the DSP smoke test.
- Keep local build artifacts out of Git.
- Do not commit `.deps/`, `build-*`, `package-*`, `.dll`, `.exe`, `.pdb`, `.lib`, or `.zip` files.
- Keep the UI simple and beginner-friendly.
- Prefer musical macro controls over technical complexity.
- Preserve the MasAri reference character unless the change is intentionally about a different preset.

## Commit style

Use clear commit messages, for example:

```text
Fix Windows installer path
Tune MasAri smart treble guard
Improve OBS release workflow
Add user install guide
```

## License of contributions

Unless explicitly stated otherwise, all contributions submitted to this repository are provided under the Apache License 2.0.
