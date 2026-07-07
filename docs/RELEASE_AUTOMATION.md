# Release automation

## Overview
This repository publishes public release assets automatically when a version tag like `v0.4.3` is pushed.

## Published assets
- Windows installer `.exe`
- Windows portable `.zip`
- Linux `.tar.gz`
- user-facing release notes

## How to cut a release
1. Update version metadata and release notes.
2. Commit changes to `main`.
3. Create and push a tag:

```bash
git tag v0.4.3
git push origin v0.4.3
```

4. GitHub Actions builds artifacts and creates the release automatically.

## Release notes source
The workflow prefers `docs/releases/<tag>.md`.
If that file does not exist, it falls back to `docs/releases/latest.md`.
