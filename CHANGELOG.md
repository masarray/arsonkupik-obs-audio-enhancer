# Changelog

All notable changes to this project are documented here.

## [Unreleased]
### Changed
- Replaced the OBS-only transition implementation with a shared, allocation-free preset and bypass transition processor used by both the plugin wrapper and standalone regression tests.
- Added an OBS-equivalent rapid transition matrix covering all factory presets, bypass changes, 44.1/48/96 kHz sample rates, 64–1024 frame blocks, and 2–20 ms preset request intervals.
- Expanded preset loudness validation from one moderate synthetic level to a five-level matrix from -18 dBFS through -1 dBFS, including RMS benefit, peak ceiling, crest-factor loss, gain-reduction, and finite-output gates.
- Made factory EQ identifiers static metadata so preset copies do not allocate strings on the realtime audio thread.

## [0.4.14] - 2026-07-15
### Changed
- Strengthened the macro response curves at moderate factory-preset values so restrained presets sound more clearly enhanced without changing the neutral point or the 0/100 endpoints.
- Added fixed, source-independent per-preset wow calibration so every factory preset delivers an immediate loudness lift without restoring the removed dynamic full-band makeup AGC.
- Calibrated all twelve factory presets to approximately +3 dB RMS benefit on the maintained 48 kHz stereo reference, with transient peak benefit guarded below +4.7 dB and no clipping.
- Expanded the preset smoke test to measure both RMS and peak benefit and fail when any factory preset falls outside the calibrated wow window.

## [0.4.13] - 2026-07-15
### Fixed
- Replaced UI/audio shared `RuntimeParams` and string access with a bounded seqlock-style atomic POD snapshot.
- Moved all DSP engine preparation, parameter application, and preset mutation to the OBS audio callback.
- Replaced preset fade-to-silence with a normalized dual-engine 10 ms crossfade.
- Kept macro EQ topology fixed during Smart Bass automation so crossing cleanup thresholds no longer resizes or resets the biquad bank.
- Replaced dynamic input-headroom and smart-makeup AGC loops with fixed calibrated headroom and restore to prevent breathing after bass transients.
- Normalized parallel-compressor mixing so correlated dry/wet lanes no longer create an intrinsic level boost.
- Cached compressor, limiter, width-detector, and gain coefficients outside per-sample hot paths.
- Reworked stereo correlation to use smoothed energy terms `E[L×R] / sqrt(E[L²]E[R²])` at control rate.
- Moved metering to block rate and added denormal protection for stateful biquads.
- Added a hardening regression test for automation discontinuity, limiter ceiling, stereo correlation, pumping stability, and zero steady-state allocations.

## [0.4.12] - 2026-07-14
### Fixed
- Recalibrated ON/OFF gain staging so Filter ON targets a tasteful wow effect instead of a hidden large volume jump.
- Added an internal -8 dB creative-chain headroom trim before EQ/color/compressor processing.
- Restored only partial headroom after the creative rack and reduced smart makeup caps so the main MasAri preset lands near +3 to +4 dB instead of extreme loudness.
- Reduced compressor makeup and parallel density caps to keep audio stable and avoid pumping while preserving the stronger v0.4.11 macro knob feel.
- Updated the DSP smoke test to catch extreme hidden volume boost regressions.

## [0.4.11] - 2026-07-14
### Changed
- Ported the stronger askp-vst style macro-to-micro mapping into the OBS engine.
- Smart Bass, Smart Treble, Vocal Body, and Stereo Magic now use center-based bipolar controls so min/max movement is clearly audible.
- Added fixed macro EQ lanes for bass, treble, vocal, cleanup, and tuck behavior while preserving crackle-safe stateful EQ retuning.
- Added true stereo narrowing support so the Stereo Magic knob can move from mono/narrow through neutral to wide.
- Kept the v0.4.10 CPU-safe smoothing and stable no-pumping compressor/makeup guard.

## [0.4.10] - 2026-07-08
### Fixed
- Tightened the crackle-free parameter patch after audit.
- Added snap-to-target and dirty-apply behavior so the OBS wrapper stops retuning once smoothed parameters have settled.
- Reduced CPU overhead during idle playback by avoiding repeated engine retunes when parameters are unchanged.
- Slowed makeup recovery and made hot-peak reduction faster to reduce audible pumping under dense music.

## [0.4.9] - 2026-07-08
### Fixed
- Fixed crackle/zipper noise when OBS sliders are moved while audio is running.
