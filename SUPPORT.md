# Support

ArSonKuPik OBS Audio Enhancer is a community-maintained open-source project. Support is provided on a best-effort basis through the public GitHub repository.

## Start here

Before opening a report:

1. Read the [README](README.md) for installation and basic usage.
2. Confirm that you are using the [latest public release](https://github.com/masarray/arsonkupik-obs-audio-enhancer/releases/latest) or the current `main` branch.
3. Restart OBS Studio after installing, updating, or replacing the plugin.
4. Review the release notes under [`docs/releases/`](docs/releases/).
5. Search existing issues for the same symptom.

For audio behavior, also read [Audio quality standards](docs/AUDIO_QUALITY.md). It documents the targets for crackle-free control movement, stable dynamics, calibrated loudness, CPU use, and stereo integrity.

## Choose the correct report type

Use GitHub's structured issue forms:

- **Bug report** — installation failures, build failures, crashes, missing filters, broken UI behavior, packaging problems, or reproducible functional defects.
- **Audio quality report** — crackle, pop, pumping, excessive loudness, distortion, weak macro response, bass or vocal problems, stereo instability, CPU spikes, or audio dropouts.
- **Feature request** — a focused product, DSP, workflow, packaging, documentation, or repository improvement.

Open the issue chooser:

https://github.com/masarray/arsonkupik-obs-audio-enhancer/issues/new/choose

Do not use a public issue for a security-sensitive vulnerability. Follow [SECURITY.md](SECURITY.md).

## Information required for every bug report

Include:

- ArSonKuPik version or commit SHA;
- OBS Studio version;
- operating system and version;
- installation method: Windows installer, portable ZIP, Linux archive, source build, or CI artifact;
- exact steps to reproduce from a clean OBS launch;
- expected behavior;
- observed behavior;
- whether OBS was restarted after installation;
- relevant screenshots or screen recordings;
- OBS log, installer log, build output, or crash details when applicable.

Remove passwords, API keys, access tokens, private usernames, personal paths, stream keys, and other sensitive data before posting logs.

## Additional information required for audio-quality reports

Audio reports should also include:

- source type: music, microphone, browser/media, podcast, game, mixed program, or test signal;
- sample rate;
- exact preset;
- Enhance value;
- Smart Bass value;
- Smart Treble value;
- Vocal Body value;
- Stereo Magic value;
- Output Trim value;
- Smart Protect state;
- bypass state;
- source fader position;
- observed input and output peak levels;
- whether the ON/bypass comparison was loudness matched;
- whether the problem occurs with more than one source;
- monitoring method: headphones, speakers, or both.

For crackle or pop reports, state whether the sound occurs:

- while a control is moving;
- when the control stops;
- at one specific value;
- while changing presets;
- only on mono or stereo sources;
- only at a particular sample rate.

For pumping or breathing reports, describe whether bass transients cause the vocal, treble, or whole mix to rise and fall afterward.

For CPU or dropout reports, attach the OBS log and describe the number of plugin instances, scene complexity, sample rate, and whether the load remains high after controls stop moving.

## Useful reproduction pattern

A high-quality report usually looks like this:

```text
Version: vX.Y.Z
OBS: X.Y.Z
OS: Windows 11 / Linux distribution
Sample rate: 48 kHz
Source: stereo music playback
Preset: exact preset name
Controls: Enhance 50, Bass 50, Treble 50, Vocal 50, Stereo 50, Output Trim 0 dB
Smart Protect: ON

Steps:
1. Start continuous playback.
2. Add ArSonKuPik Smart Enhancer to the source.
3. Move Smart Bass from 0 to 100.
4. Hear a short crackle near value 70.
5. Return the control to 50; the crackle stops.

Expected: smooth tonal change without discontinuity.
Observed: one repeatable click during the sweep.
```

## Installation support

### Windows installer

- Close OBS before running the installer.
- Run the setup executable with Administrator privileges.
- Restart OBS after installation.
- The standard plugin location is:

```text
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer
```

### Windows portable ZIP

Copy the packaged `arsonkupik-obs-audio-enhancer` directory into:

```text
C:\ProgramData\obs-studio\plugins\
```

Do not copy a full development repository into the OBS plugin folder.

### Linux

Linux package locations vary by distribution and OBS installation method. Include the distribution, package source, destination path, and OBS log in any installation report.

## Build support

For a local Windows plugin build, use:

```text
build_plugin_single_click.bat
```

For the standalone DSP smoke test:

```bash
cmake -S . -B build-test -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-test --config Release
```

Attach the complete configure and build output when reporting a build failure. A screenshot of only the final error line is usually insufficient.

## Scope and expectations

The project does not guarantee individual response times, compatibility with unsupported OBS versions, or support for unofficial repackaged binaries.

Reports may be closed when they:

- cannot be reproduced and lack requested details;
- concern an outdated release already fixed in the latest version;
- use modified or unofficial binaries without source details;
- contain only a subjective statement without settings or reproduction steps;
- duplicate an existing issue;
- disclose sensitive information publicly.

Maintainers may ask for a smaller reproduction, additional logs, or a test build before accepting a fix.

## Copyright and privacy

Share only audio, screenshots, logs, and recordings that you are permitted to publish. Prefer short original, public-domain, or properly licensed test material. Remove personal information before posting.
