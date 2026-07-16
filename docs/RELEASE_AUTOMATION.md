# Release automation

## Overview

ArSonKuPik publishes public release assets only when an annotated semantic-version tag matching `vX.Y.Z` is pushed.

The release workflow is intentionally separate from GitHub Pages deployment:

- `.github/workflows/release.yml` validates, builds, checksums, and publishes tagged release assets.
- `.github/workflows/pages.yml` deploys the landing site from `main` when files under `docs/site/**` or `assets/images/**` change.

The release workflow has no manual-dispatch path. This prevents a branch name such as `main` from being interpreted as a product version.

## Reproducible OBS dependency

Windows CI and tagged Windows releases build against the OBS Studio version stored in:

```text
cmake/OBS_VERSION.txt
```

Changing the OBS dependency requires an explicit reviewed commit. Rebuilding the same ArSonKuPik source therefore does not silently switch to a newer OBS release.

Local Windows builds use the pinned version by default. `-OBSVersion latest` remains available only as an intentional compatibility experiment and must not be used for official release assets.

## Mandatory release validation

Before any platform packaging starts, the release workflow verifies:

1. the tag matches `vX.Y.Z`;
2. the tag exactly matches the version in `CMakeLists.txt`;
3. `docs/releases/<tag>.md` exists;
4. `cmake/OBS_VERSION.txt` contains a valid pinned version;
5. the multi-level loudness matrix passes;
6. realtime engine hardening passes;
7. preset and bypass transition hardening passes.

Windows and Linux packaging jobs depend on this validation job. A failing DSP gate cannot publish a release.

## Published assets

Each successful tagged release publishes:

- Windows installer `.exe`;
- Windows portable `.zip`;
- Linux `.tar.gz`;
- `BUILD-METADATA.txt` with source commit and OBS dependency;
- `SHA256SUMS.txt` covering every binary asset and the metadata file;
- user-facing release notes from `docs/releases/<tag>.md`.

## Release readiness checklist

Before creating a tag, confirm:

- [ ] `CMakeLists.txt` contains the intended project version.
- [ ] `CHANGELOG.md` contains a dated entry for the release.
- [ ] `docs/releases/vX.Y.Z.md` contains user-facing release notes.
- [ ] `docs/releases/latest.md` matches the intended release.
- [ ] landing-page structured data reports the intended version.
- [ ] `cmake/OBS_VERSION.txt` points to the tested OBS dependency.
- [ ] multi-level loudness, realtime hardening, and transition tests pass.
- [ ] Windows and Linux CI builds pass on `main`.
- [ ] native OBS listening checks pass with music, voice, hot mastered audio, rapid controls, preset switching, and bypass transitions.
- [ ] no build outputs, installers, archives, or dependency caches are staged in Git.

## How to cut a release

Start from an up-to-date local `main` branch:

```bash
git checkout main
git pull origin main
git status
```

Confirm the version before tagging:

```bash
grep "project(arsonkupik-obs-audio-enhancer VERSION" CMakeLists.txt
cat cmake/OBS_VERSION.txt
```

Create an annotated tag from the validated `main` commit:

```bash
git tag -a vX.Y.Z -m "ArSonKuPik OBS Audio Enhancer vX.Y.Z"
git push origin vX.Y.Z
```

GitHub Actions then runs:

1. tag, version, release-note, dependency, and DSP validation;
2. Windows installer and portable build;
3. Linux package build;
4. release-asset flattening, metadata generation, and SHA-256 checksums;
5. GitHub Release publication after all dependencies succeed.

## Verifying downloaded assets

On Linux:

```bash
sha256sum -c SHA256SUMS.txt
```

On Windows PowerShell, compare the published value with:

```powershell
Get-FileHash .\ArSonKuPik-OBS-Audio-Enhancer-Setup-vX.Y.Z.exe -Algorithm SHA256
```

The checksum confirms file integrity. It is not a substitute for operating-system code signing.

## Failed or incorrect tags

A tag keeps the workflow and source snapshot from the commit it references. Correcting `main` does not alter an existing tag.

For an unpublished or clearly invalid tag:

```bash
git tag -d vX.Y.Z
git push origin :refs/tags/vX.Y.Z
git tag -a vX.Y.Z -m "ArSonKuPik OBS Audio Enhancer vX.Y.Z"
git push origin vX.Y.Z
```

When a public release already exists, publish a new patch version instead of rewriting release history.

## GitHub Pages

Landing-page changes are deployed from `main` by the dedicated Pages workflow. Tagged releases never deploy the Pages environment and require no Pages or identity-token permissions.
