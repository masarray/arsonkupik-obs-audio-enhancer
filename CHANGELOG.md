# Changelog

All notable changes to this project are documented here.

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
- Reworked EQ rebuild to preserve existing biquad history and only retune coefficients during normal knob automation.
- Replaced Butterworth Q allocation with static tables so repeated retunes do not allocate in the audio path.
- Added 80 ms block-level smoothing for continuous OBS macro controls.
- Added equal-power preset-switch fade around topology changes to avoid pop/click when changing presets.

## [0.4.5] - 2026-07-07
### Fixed
- Fixed Linux release workflow link error by enabling position-independent code for the static DSP library.
- Added Linux plugin build smoke test to CI.
- Set Linux CMake release configuration explicitly.

## [0.4.4] - 2026-07-07
### Changed
- Cleaned public repository helper scripts.
- Kept only the Windows batch files that are actually useful for normal local use:
  - `build_plugin_single_click.bat`
  - `install_plugin_windows.bat`
  - `scripts/find-cmake.bat` because the build helper uses it internally
- Improved Windows installer logic so the standard OBS plugin folder can already exist without producing a confusing warning.
- Installer now detects a typical OBS installation and installs into the standard ProgramData plugin path.
- Confirmed project licensing as GPL-3.0.

## [0.4.3] - 2026-07-07
### Added
- Public release automation for Windows installer, Windows ZIP, and Linux package artifacts.
- GitHub Pages landing site and README assets.
- Public, user-facing release notes.

### Changed
- Repository cleaned up for public presentation.
- README rewritten for public users and contributors.
- Project license updated to GPL-3.0.
