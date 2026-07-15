# Release automation

## Overview

ArSonKuPik publishes public release assets automatically when an annotated version tag matching `v*` is pushed.

The release workflow is intentionally separate from GitHub Pages deployment:

- `.github/workflows/release.yml` builds and publishes tagged release assets.
- `.github/workflows/pages.yml` deploys the landing site from `main` when files under `docs/site/**` or `assets/images/**` change.

A release tag must never be used to deploy the Pages environment.

## Published assets

Each successful tagged release publishes:

- Windows installer `.exe`
- Windows portable `.zip`
- Linux `.tar.gz`
- user-facing release notes

## Release readiness checklist

Before creating a tag, confirm that all public version references and release content are synchronized:

- [ ] `CMakeLists.txt` contains the intended project version.
- [ ] `CHANGELOG.md` contains a dated entry for the release.
- [ ] `docs/releases/vX.Y.Z.md` contains user-facing release notes.
- [ ] `docs/releases/latest.md` matches the intended release.
- [ ] README and installation documentation remain accurate.
- [ ] DSP smoke tests pass.
- [ ] Windows and Linux CI builds pass on `main`.
- [ ] No build outputs, installers, archives, or dependency caches are staged in Git.

## How to cut a release

Start from an up-to-date local `main` branch:

```bash
git checkout main
git pull origin main
git status
```

Commit and push the synchronized release metadata before tagging:

```bash
git add CMakeLists.txt CHANGELOG.md docs/releases/vX.Y.Z.md docs/releases/latest.md
git commit -m "Prepare ArSonKuPik vX.Y.Z"
git push origin main
```

Create an annotated tag from the updated `main` commit:

```bash
git tag -a vX.Y.Z -m "ArSonKuPik OBS Audio Enhancer vX.Y.Z"
git push origin vX.Y.Z
```

GitHub Actions then runs three release jobs:

1. Build Windows release assets.
2. Build the Linux release archive.
3. Publish the GitHub Release after both platform builds succeed.

## Release notes source

The workflow first looks for:

```text
docs/releases/<tag>.md
```

For tag `vX.Y.Z`, the expected file is:

```text
docs/releases/vX.Y.Z.md
```

When that file is missing, the workflow falls back to `docs/releases/latest.md`. The fallback should be treated as recovery behavior, not the normal release process.

## Failed or incorrect tags

Do not repeatedly rerun an old tag after the workflow or release content has been corrected on `main`; a tag keeps the workflow snapshot and source tree from the commit it references.

For an unpublished or clearly invalid tag, delete it and recreate it only after confirming the corrected commit:

```bash
git tag -d vX.Y.Z
git push origin :refs/tags/vX.Y.Z
git tag -a vX.Y.Z -m "ArSonKuPik OBS Audio Enhancer vX.Y.Z"
git push origin vX.Y.Z
```

When a public release already exists, prefer publishing a new patch version instead of rewriting release history.

## GitHub Pages

Landing-page changes are deployed from `main` by the dedicated Pages workflow. To trigger a clean deployment, commit a legitimate change under:

```text
docs/site/**
assets/images/**
```

The tagged release workflow requires only `contents: write`; it must not request Pages or identity-token permissions.
