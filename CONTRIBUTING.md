# Contributing to ArSonKuPik OBS Audio Enhancer

Thanks for helping improve ArSonKuPik. Contributions are welcome when they keep the project practical for real-time OBS use, preserve a simple user experience, and include enough evidence to review the change safely.

## Before starting

- Read the [README](README.md).
- Read [Audio quality standards](docs/AUDIO_QUALITY.md) before changing DSP, presets, gain staging, smoothing, stereo behavior, or the OBS audio wrapper.
- Search existing issues and pull requests.
- Open an issue before large features, architecture changes, new dependencies, or user-visible DSP redesigns.
- Use the structured [issue forms](https://github.com/masarray/arsonkupik-obs-audio-enhancer/issues/new/choose).

Small fixes with an obvious scope may go directly to a pull request.

## Project priorities

Changes should support these product goals:

- crackle-free continuous control movement;
- stable preset switching without hard pops or clicks;
- practical CPU use for streaming and recording;
- stable dynamics without obvious pumping or breathing;
- calibrated perceived loudness rather than extreme hidden gain;
- audible and useful macro controls;
- stable center focus and acceptable mono compatibility;
- straightforward installation and public documentation;
- professional public releases for Windows and Linux.

## Development setup

The project uses CMake and C++17.

### Standalone DSP build

```bash
cmake -S . -B build-test -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-test --config Release
```

Run the smoke test on Linux:

```bash
./build-test/arsonkupik_dsp_smoke
```

With a multi-configuration Windows generator, the executable is normally under:

```text
build-test\Release\arsonkupik_dsp_smoke.exe
```

### Windows OBS plugin build

Use the maintained helper:

```text
build_plugin_single_click.bat
```

Install the locally generated ProgramData-ready package with:

```text
install_plugin_windows.bat
```

Do not commit generated build directories, downloaded dependencies, installers, archives, plugin binaries, or package output.

## Branch and pull-request workflow

1. Create a focused branch from current `main`.
2. Make the smallest coherent change that solves the documented problem.
3. Add or update tests.
4. Update user-facing documentation when behavior changes.
5. Run the relevant local build and smoke tests.
6. Push the branch and open a pull request.

A pull request should contain one primary purpose. Avoid mixing DSP tuning, UI redesign, workflow cleanup, documentation overhaul, and unrelated refactoring in one change.

## Coding expectations

- Keep the code compatible with the project's C++17 requirement.
- Prefer clear, bounded, maintainable code over clever abstractions.
- Preserve existing public identifiers and user-facing names unless a migration is intentional and documented.
- Clamp user-controlled and calculated DSP parameters to valid ranges.
- Handle mono, stereo, null, and unsupported-channel cases safely.
- Avoid unnecessary new dependencies.
- Keep comments focused on non-obvious constraints and design reasons.
- Do not add secrets, tokens, private URLs, generated credentials, or personal environment paths.
- Keep source and runtime strings portable and UTF-8 safe.

## Real-time audio-thread rules

DSP and wrapper changes require extra care:

- no heap allocation in steady-state processing when it can be prepared ahead of time;
- no file access, network access, logging, waiting, or mutex contention in the audio callback;
- no filter-state reset for an ordinary coefficient change;
- no per-mouse-event rebuild when a block-rate smoothed update is sufficient;
- no unbounded loops or source-dependent memory growth;
- no unnecessary per-sample calculation that can be performed per block;
- no hidden output boost used to make a tonal change appear better;
- no global oversampling without measurements and a documented audible benefit.

When filter topology must change, preserve state where possible and use a click-safe transition when it cannot be preserved.

## Requirements for DSP and preset changes

A DSP pull request must explain:

- the listening problem;
- the intended audible result;
- the source types used for evaluation;
- the exact presets and controls affected;
- expected CPU impact;
- expected gain impact;
- crackle, pumping, clipping, and stereo risks;
- objective test results;
- subjective listening results;
- known trade-offs.

Use both native ON/OFF and loudness-matched comparisons. A result that only sounds better because it is much louder is not sufficient.

Test at least:

- 44.1 kHz and 48 kHz when practical;
- mono and stereo input;
- continuous control sweeps;
- repeated preset changes;
- bass-heavy material;
- spoken voice;
- already-wide stereo material;
- mono fold-down for stereo changes.

## Tests and release gates

At minimum, run the standalone DSP smoke test for any engine or preset change.

A DSP-affecting change should not be merged when it introduces:

- non-finite output;
- sample clipping in maintained test scenarios;
- audible crackle or zipper noise;
- hard preset-switch pops;
- obvious full-band pumping;
- an extreme Filter ON level jump;
- unstable stereo center or severe mono cancellation;
- a significant CPU increase without evidence of value.

Add a regression test when fixing a reproducible bug. A fix without a test should explain why automated coverage is not practical.

## Documentation requirements

Update the relevant public files when behavior changes:

- `README.md` for user-facing capabilities or installation changes;
- `CHANGELOG.md` for notable changes;
- `docs/releases/vX.Y.Z.md` and `docs/releases/latest.md` for release-visible behavior;
- `docs/AUDIO_QUALITY.md` when quality targets or validation methods change;
- build and packaging docs for workflow changes;
- landing-page content for major public product changes.

Documentation should speak to users and contributors. Avoid internal audit narration, unsupported marketing claims, or measurements that cannot be reproduced.

## Commit and repository hygiene

- Use descriptive commit messages.
- Do not commit build artifacts or local dependency caches.
- Do not rewrite public release history to conceal a mistake; use a new patch release when appropriate.
- Keep `.gitignore` aligned with any new generated paths.
- Do not include copyrighted audio test material unless redistribution is permitted.
- Do not commit third-party code without confirming license compatibility with GPL-3.0.

## Pull-request description

Include:

- problem statement;
- solution summary;
- changed files or subsystems;
- local test commands and results;
- Windows/Linux status;
- screenshots for UI or landing-page work;
- audio test details for DSP work;
- migration or compatibility notes;
- release-note impact.

## Pull-request checklist

- [ ] The change has one clear purpose.
- [ ] The branch is based on current `main`.
- [ ] Local builds pass for the affected targets.
- [ ] The standalone DSP smoke test passes when DSP is affected.
- [ ] Continuous controls remain crackle-free.
- [ ] Preset switching remains click-safe.
- [ ] Dynamics do not introduce obvious pumping.
- [ ] Gain staging does not create an extreme hidden boost.
- [ ] Stereo changes preserve center focus and acceptable mono fold-down.
- [ ] CPU impact is acceptable and documented.
- [ ] Tests cover the regression or new behavior.
- [ ] User-facing documentation is updated.
- [ ] Changelog and release notes are updated when required.
- [ ] No binaries, archives, build output, secrets, or private data are committed.

## Security issues

Do not open a public pull request or issue for an undisclosed vulnerability. Follow [SECURITY.md](SECURITY.md).

## Code of conduct

Be respectful, specific, and focused on the technical work. Critique code and behavior rather than people. Harassment, personal attacks, or deliberate disruption are not accepted.
