# Changelog

All notable changes to this project are documented here.

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
