# Build Troubleshooting

## `obsconfig.h`: No such file or directory

This happens when the one-click build uses raw OBS Studio source headers. `libobs/obs-config.h` includes a generated file named `obsconfig.h`. That generated file normally exists only after OBS itself is configured by CMake.

Patch v0.2.4 creates a small fallback `libobs/obsconfig.h` inside the downloaded OBS source tree before configuring this plugin.

Fix:

```bat
clean_build_cache.bat
build_plugin_single_click.bat
```

If you already have `.deps` from an older build, you can keep it. The v0.2.4 script will create the missing header inside `.deps\obs-source\...\libobs\obsconfig.h`.

## PowerShell parser error in `package-windows-programdata.ps1`

Patch v0.2.4 fixes the candidate path array syntax. The previous script accidentally used comma-separated `Join-Path` calls without wrapping each call in parentheses.

## Build continues to install/package after compile failure

Patch v0.2.4 checks `$LASTEXITCODE` after native CMake/lib commands, so a failed compile stops immediately and does not try to install a missing DLL.


## v0.2.4 PowerShell parser hotfix

If PowerShell reports `Variable reference is not valid` around `$LASTEXITCODE:`, update to v0.2.4. PowerShell can parse a colon immediately after a variable name as a scoped variable reference. The build script now uses `${LASTEXITCODE}:` in the error message.
