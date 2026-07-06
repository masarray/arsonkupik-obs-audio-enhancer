# Cross-platform release packaging

ArSonKuPik OBS Audio Enhancer should expose two kinds of downloads per platform:

1. Beginner-friendly installer package.
2. Advanced/manual package for users who want to inspect and copy the OBS plugin folder manually.

## Windows

Current packaged target.

Release assets:

```text
ArSonKuPik-OBS-Audio-Enhancer-Setup-vX.Y.Z.exe
ArSonKuPik-OBS-Audio-Enhancer-Setup-vX.Y.Z.exe.sha256
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-vX.Y.Z.zip
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-vX.Y.Z.zip.sha256
```

User flow:

```text
Close OBS Studio > run EXE installer > restart OBS Studio
```

Advanced flow:

```text
Extract ZIP > run install_windows.bat as Administrator > restart OBS Studio
```

The EXE is built with Inno Setup from:

```text
packaging/windows/arsonkupik-obs-audio-enhancer.iss
```

## macOS

Planned target.

Recommended release assets:

```text
ArSonKuPik-OBS-Audio-Enhancer-macOS-universal-vX.Y.Z.pkg
ArSonKuPik-OBS-Audio-Enhancer-macOS-universal-vX.Y.Z.pkg.sha256
ArSonKuPik-OBS-Audio-Enhancer-macOS-universal-vX.Y.Z.zip
ArSonKuPik-OBS-Audio-Enhancer-macOS-universal-vX.Y.Z.zip.sha256
```

Notes:

- Prefer a signed and notarized `.pkg` for beginner users.
- Provide `.zip` for advanced/manual install.
- Build architecture can be universal, arm64, or x86_64 depending on the OBS target and CI capacity.
- macOS release quality requires code signing and notarization secrets.

## Linux

Planned target.

Recommended release assets:

```text
ArSonKuPik-OBS-Audio-Enhancer-Linux-x86_64-vX.Y.Z.tar.gz
ArSonKuPik-OBS-Audio-Enhancer-Linux-x86_64-vX.Y.Z.tar.gz.sha256
ArSonKuPik-OBS-Audio-Enhancer-Linux-x86_64-vX.Y.Z.deb
ArSonKuPik-OBS-Audio-Enhancer-Linux-x86_64-vX.Y.Z.deb.sha256
```

Notes:

- Provide `.tar.gz` first because it is the simplest manual package.
- Add `.deb` later for Ubuntu/Debian users.
- Flatpak OBS may require a different plugin install path or separate packaging guidance.
- Linux OBS plugin compatibility can vary by distribution and OBS/libobs version.

## Release page layout

Recommended release copy:

```markdown
## Recommended downloads

### Windows
- Normal users: download `ArSonKuPik-OBS-Audio-Enhancer-Setup-vX.Y.Z.exe`
- Advanced users: download `ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-vX.Y.Z.zip`

### macOS
Coming soon.

### Linux
Coming soon.
```

## Roadmap

1. Windows EXE installer and ZIP package.
2. Linux `.tar.gz` package.
3. Linux `.deb` package.
4. macOS `.pkg` package.
5. macOS signing and notarization.
