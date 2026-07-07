# GitHub build and release automation

The repository uses three workflows:

```text
.github/workflows/ci.yml
.github/workflows/release.yml
.github/workflows/pages.yml
```

## Trigger CI

Push to `main` or open a pull request.

## Trigger release

Push a version tag:

```powershell
git tag -a v0.4.4 -m "ArSonKuPik OBS Audio Enhancer v0.4.4"
git push origin v0.4.4
```

Release output:

```text
Windows installer .exe
Windows portable .zip
Linux .tar.gz
GitHub Release notes
```

## Landing page

GitHub Pages is deployed from `docs/site` through GitHub Actions.
