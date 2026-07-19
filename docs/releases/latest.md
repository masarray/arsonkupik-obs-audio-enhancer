# ArSonKuPik OBS Audio Enhancer v0.4.15

## Highlights
- Validates preset loudness from moderate material through hot/mastered input levels instead of relying on one synthetic level.
- Uses the same allocation-free preset and bypass transition processor in OBS and standalone regression tests.
- Reduces control-movement CPU work by rebuilding only DSP subsystems affected by the changed parameter.
- Adds a true settled hard-bypass path with exact pass-through audio.
- Strengthens release integrity with CTest, sanitizers, immutable GitHub Action pins, checksums, and pinned OBS build dependencies.

## Engine changes
- Stereo Magic changes rebuild only the width subsystem.
- Output Trim changes update output gain and limiter configuration without rebuilding EQ, color, width, or compression.
- Bypass-only changes do not rebuild creative DSP configuration.
- Color, compressor, and width processors remain state-warm at neutral positions; audible contribution reaches zero through mix and gain rather than abrupt enable thresholds.
- After the bypass smoother reaches its dry target, subsequent blocks skip EQ, compression, color, width, output gain, and limiting while meters remain updated.
- Factory loudness calibration is stored in each stable preset instead of depending on factory-preset vector order.
- The MasAri factory preset is the single source for default standalone and OBS parameter values.

## Validation
- Multi-level loudness coverage spans -18, -12, -6, -3, and -1 dBFS input peaks.
- Transition coverage spans all factory presets, bypass changes, 44.1/48/96 kHz sample rates, 64–1024 frame blocks, and rapid 2–20 ms request intervals.
- Registered CTest gates cover preset loudness/clipping, realtime hardening, and preset/bypass transitions.
- AddressSanitizer and UndefinedBehaviorSanitizer run in CI with leak detection and fail-fast settings.
- Regression tests verify default consistency, selective subsystem rebuild counters, exact hard-bypass pass-through, and zero allocations during settled bypass.

## Release and supply-chain integrity
- GitHub Actions dependencies are pinned to immutable full commit SHAs and monitored by weekly Dependabot updates.
- Tagged releases validate semantic version consistency, release notes, the pinned OBS dependency, and all registered DSP tests before platform packaging begins.
- Windows installer, Windows portable ZIP, and Linux archive are accompanied by `SHA256SUMS.txt` and `BUILD-METADATA.txt`.
- GitHub Release publishing uses the authenticated GitHub CLI rather than a third-party publishing action.

## Channel policy
ArSonKuPik is a stereo/front-LR processor. On multichannel OBS sources, channels after front left/right are passed through unchanged to preserve source data until a linked multichannel design is implemented and validated.
