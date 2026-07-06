# GitHub public repo + auto build

This repo includes two GitHub Actions workflows:

- `.github/workflows/ci-dsp.yml`
  - Builds and runs the standalone DSP smoke test on Windows and Ubuntu.
  - Does not need OBS/libobs.

- `.github/workflows/build-windows-plugin.yml`
  - Builds a Windows x64 OBS plugin package.
  - Downloads OBS Studio portable + OBS source headers.
  - Generates a local `obs.lib` import library from `obs.dll`.
  - Builds `arsonkupik-obs-audio-enhancer.dll`.
  - Uploads a ready-to-install ZIP artifact.
  - If you push a tag such as `v0.2.0`, it creates/updates a GitHub Release with the ZIP attached.

## Publish as a public GitHub repo

### Option A - one click from Windows

1. Install Git.
2. Install GitHub CLI.
3. Run:

```bat
publish_public_repo.bat
```

The script will run `gh auth login` if needed, create a public repository, push the source, and open the repo page.

### Option B - manual commands

```bat
git init
git branch -M main
git add .
git commit -m "Initial ArSonKuPik OBS native audio enhancer"
gh auth login
gh repo create arsonkupik-obs-audio-enhancer --public --source=. --remote=origin --push
```

## Trigger auto build

After pushing to `main`, GitHub Actions will run automatically.

To create a public release with the ready-to-install plugin ZIP:

```bat
git tag v0.2.0
git push origin v0.2.0
```

Then open the GitHub repository → Releases → download:

```text
arsonkupik-obs-audio-enhancer-windows-x64.zip
```

## Install artifact into OBS

Extract the ZIP. Copy the folder:

```text
arsonkupik-obs-audio-enhancer
```

into:

```text
C:\ProgramData\obs-studio\plugins\
```

Final layout should be:

```text
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\bin\64bit\arsonkupik-obs-audio-enhancer.dll
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\data\locale\en-US.ini
```

Restart OBS, then add the filter:

```text
Audio Source → Filters → + → ArSonKuPik Smart Enhancer
```

## Local one-click build

Run:

```bat
build_plugin_single_click.bat
```

It will:

1. Find Visual Studio C++ tools.
2. Download OBS portable + source headers.
3. Generate a local OBS import library.
4. Build the plugin DLL.
5. Create the ProgramData-ready package.

Output:

```text
package-programdata\arsonkupik-obs-audio-enhancer\
```

Then run:

```bat
install_plugin_windows.bat
```

or copy the package folder manually into `C:\ProgramData\obs-studio\plugins\`.

## Notes

The GitHub Actions plugin build intentionally avoids requiring a preinstalled OBS SDK. It uses the OBS release portable package and OBS source headers for repeatable CI builds. If OBS changes release asset naming or binary exports in a future version, pin the workflow input to a known-good OBS version/tag.
