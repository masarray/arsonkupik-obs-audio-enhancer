# Changelog

All notable changes to this project are documented here.

## [0.4.6] - 2026-07-07
### Fixed
- Fixed the release workflow badge/status by removing GitHub Pages deployment from tag-based releases.
- Release workflow now only builds Windows assets, builds Linux assets, and publishes GitHub Releases.
- GitHub Pages deployment remains handled by the separate Pages workflow from `main`.

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
