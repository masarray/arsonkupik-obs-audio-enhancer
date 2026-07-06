# Release and package workflow

The repository builds Windows x64 OBS plugin packages through GitHub Actions.

Workflow file:

```text
.github/workflows/build-windows-plugin.yml
```

## What the workflow produces

For a tag such as `v0.3.1`, the release page should include:

```text
ArSonKuPik-OBS-Audio-Enhancer-Setup-v0.3.1.exe
ArSonKuPik-OBS-Audio-Enhancer-Setup-v0.3.1.exe.sha256
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-v0.3.1.zip
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-v0.3.1.zip.sha256
```

## User install flow

Normal Windows users should download the `.exe` installer:

```text
Close OBS Studio > run EXE installer > restart OBS Studio
```

Advanced users can download the ZIP package:

```text
Extract ZIP > run install_windows.bat as Administrator > restart OBS Studio
```

## Automatic release from tag

From the local repo:

```powershell
git pull origin main
git tag v0.3.1
git push origin v0.3.1
```

The workflow will build the plugin, package the installer EXE, package the manual ZIP, generate SHA256 checksums, and create/update the GitHub Release.

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
release_tag = v0.3.1
```

The workflow creates or updates that GitHub Release and uploads the installer EXE, manual ZIP, and checksum files.

## Platform roadmap

See:

```text
docs/CROSS_PLATFORM_RELEASES.md
```
