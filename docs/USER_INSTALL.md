# User install guide

This document is for end users who only want to install the compiled OBS plugin.

## Recommended Windows install

Open the latest GitHub Release and download the installer:

```text
ArSonKuPik-OBS-Audio-Enhancer-Setup-vX.Y.Z.exe
```

Install:

1. Close OBS Studio.
2. Run the `.exe` installer.
3. Approve Administrator permission.
4. Start OBS Studio again.
5. Add the filter from:

```text
Audio Source > Filters > + > ArSonKuPik Smart Enhancer
```

## Advanced/manual Windows install

Download the ZIP package:

```text
ArSonKuPik-OBS-Audio-Enhancer-Windows-x64-vX.Y.Z.zip
```

Then:

1. Close OBS Studio.
2. Extract the ZIP file.
3. Run `install_windows.bat` as Administrator, or copy the plugin folder manually.
4. Restart OBS Studio.

## Default setting

```text
Preset: MasAri
Manual macro tuning: OFF
Smart Protect: ON
Bypass: OFF
```

## Uninstall

Use Windows Apps settings if installed with the `.exe` installer, or run:

```text
uninstall_windows.bat
```

## Manual install path

Copy the folder:

```text
arsonkupik-obs-audio-enhancer
```

into:

```text
C:\ProgramData\obs-studio\plugins\
```

Final layout:

```text
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\bin\64bit\arsonkupik-obs-audio-enhancer.dll
C:\ProgramData\obs-studio\plugins\arsonkupik-obs-audio-enhancer\data\locale\en-US.ini
```
