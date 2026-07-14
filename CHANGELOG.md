# Changelog

All notable changes to this project are documented here.

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
