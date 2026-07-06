# Release and package workflow

The repository builds a ready-to-install Windows x64 OBS plugin package through GitHub Actions.

Workflow file:

```text
.github/workflows/build-windows-plugin.yml
```

## What the workflow produces

The release ZIP is named like:

```text
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-v0.2.5.zip
```

Inside the ZIP:

```text
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64/
├─ install_windows.bat
├─ uninstall_windows.bat
├─ README_INSTALL.txt
├─ VERSION.txt
└─ arsonkupik-obs-audio-enhancer/
   ├─ bin/64bit/arsonkupik-obs-audio-enhancer.dll
   └─ data/locale/en-US.ini
```

## Automatic release from tag

From the local repo:

```powershell
git pull origin main
git tag v0.2.5
git push origin v0.2.5
```

The workflow will build the plugin, package the installer ZIP, generate a `.sha256` checksum, and create/update the GitHub Release.

## Manual workflow run without publishing release

Use this to test build artifacts:

```text
GitHub > Actions > Build and Release Windows OBS Plugin > Run workflow
publish_release = false
```

The result appears under the workflow run Artifacts.

## Manual workflow run with release publishing

```text
GitHub > Actions > Build and Release Windows OBS Plugin > Run workflow
publish_release = true
release_tag = v0.2.5
```

The workflow creates or updates that GitHub Release and uploads:

```text
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-v0.2.5.zip
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-v0.2.5.zip.sha256
```

## User install flow

End users only need to:

1. Download the ZIP from GitHub Releases.
2. Extract it.
3. Double-click `install_windows.bat`.
4. Restart OBS Studio.
5. Add the filter from `Audio Source > Filters > + > ArSonKuPik Smart Enhancer`.
