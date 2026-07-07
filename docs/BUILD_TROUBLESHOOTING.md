# Build troubleshooting

## CMake not found

Run `build_plugin_single_click.bat`. It calls `scripts/find-cmake.bat`, which checks PATH, common standalone CMake locations, and Visual Studio bundled CMake.

## Visual Studio C++ toolchain not found

Install Visual Studio 2022 and enable:

```text
Desktop development with C++
```

## `obsconfig.h`: No such file or directory

The one-click build creates a fallback generated header inside the downloaded OBS source tree. If an old dependency cache causes problems, delete `.deps` and rerun:

```bat
build_plugin_single_click.bat
```

## Install copy failed

Close OBS Studio, then rerun `install_plugin_windows.bat` as Administrator.
