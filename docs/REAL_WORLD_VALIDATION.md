# Real-world DSP validation matrix

This document defines the P0 regression gates added after the v0.4.14 preset calibration audit. The purpose is to prevent a preset from passing only because it was tuned to one low-level synthetic signal, and to test the same dual-engine transition path used by the OBS plugin.

## Multi-level loudness matrix

Every factory preset is rendered at 48 kHz with transient stereo program material normalized to these input sample-peak levels:

- -18 dBFS
- -12 dBFS
- -6 dBFS
- -3 dBFS
- -1 dBFS

The test records:

- input and output sample peak;
- input and output RMS;
- ON/bypass RMS and peak benefit;
- crest-factor loss;
- 95th-percentile reported gain reduction;
- finite output and output-ceiling status.

Moderate sources retain the calibrated approximately +3 dB RMS wow target. Hot sources are allowed to deliver less level lift because available headroom is limited; they must remain finite, respect the output ceiling, and stay inside bounded crest-factor and gain-reduction limits. The -1 dBFS row is a stress case rather than a promise of an additional +3 dB output level.

## OBS-equivalent transition matrix

The OBS wrapper and standalone test use the same `ArSonKuPikTransitionProcessor`. The matrix covers:

- all factory presets;
- bypass changes during active audio;
- preset requests while a 10 ms transition is still active;
- 44.1, 48, and 96 kHz sample rates;
- 64, 128, 256, 480, and 1024 frame blocks;
- 2, 10, and 20 ms preset-request intervals.

Acceptance criteria:

- no NaN or infinity values;
- no output above the maintained full-scale safety bound;
- maximum sample-jump ratio no greater than 1.10 times the worst static-preset reference;
- zero heap allocations after processor initialization.

A third preset request received during an active transition is intentionally queued until the current 10 ms blend finishes. This avoids restarting from an arbitrary intermediate mixture; the latest requested target is applied on the following audio block.

## Run locally

```bash
cmake -S . -B build-test -DBUILD_OBS_PLUGIN=OFF -DBUILD_STANDALONE_TESTS=ON
cmake --build build-test --config Release
./build-test/arsonkupik_dsp_smoke
./build-test/arsonkupik_dsp_hardening
./build-test/arsonkupik_transition_hardening
```

With a multi-configuration Windows generator, the executables are normally under `build-test/Release/`.

## Validation boundary

These automated tests are regression gates, not a complete perceptual model. A DSP-affecting release still requires listening in OBS with music, speech, bass-heavy material, already-loud mastered sources, mono material, true stereo material, rapid control movement, repeated preset changes, headphones, and speakers.
