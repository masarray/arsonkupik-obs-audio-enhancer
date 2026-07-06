param(
  [string]$OBSVersion = "latest",
  [string]$BuildDir = "build-obs-release",
  [string]$Config = "Release",
  [string]$DepsDir = ".deps",
  [string]$CMakeExe = "",
  [switch]$SkipDownload,
  [switch]$InstallToProgramData
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

function Invoke-NativeChecked {
  param(
    [Parameter(Mandatory=$true)][string]$Exe,
    [Parameter(ValueFromRemainingArguments=$true)][string[]]$Args
  )
  & $Exe @Args
  if ($LASTEXITCODE -ne 0) {
    throw "Command failed with exit code ${LASTEXITCODE}: $Exe $($Args -join ' ')"
  }
}

function Write-ObsConfigStub {
  param(
    [Parameter(Mandatory=$true)][string]$ObsSourcePath,
    [Parameter(Mandatory=$true)][string]$Tag
  )

  $libobsDir = Join-Path $ObsSourcePath "libobs"
  $obsconfig = Join-Path $libobsDir "obsconfig.h"
  if (Test-Path $obsconfig) { return }

  Write-Host "Creating fallback generated header: $obsconfig"
  @"
#pragma once

#define OBS_DATA_PATH "data"
#define OBS_PLUGIN_PATH "obs-plugins"
#define OBS_PLUGIN_DESTINATION "obs-plugins/64bit"
#define OBS_INSTALL_PREFIX "."
#define OBS_RELEASE_CANDIDATE 0
#define OBS_BETA 0
"@ | Set-Content -Encoding ASCII $obsconfig
}

$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
  if ([string]::IsNullOrWhiteSpace($CMakeExe)) {
    $cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCmd) {
      throw "CMake was not found. Use build_plugin_single_click.bat so it can auto-detect Visual Studio bundled CMake, or install standalone CMake and add it to PATH."
    }
    $CMakeExe = $cmakeCmd.Source
  }
  if (-not (Test-Path $CMakeExe)) {
    throw "CMake executable was not found: $CMakeExe"
  }
  Write-Host "Using CMake: $CMakeExe"
  if (-not (Get-Command dumpbin -ErrorAction SilentlyContinue) -or -not (Get-Command lib -ErrorAction SilentlyContinue)) {
    throw "MSVC tools dumpbin/lib were not found. Run this from a Visual Studio x64 Developer Command Prompt, or use build_plugin_single_click.bat."
  }

  $depsRoot = Join-Path $root $DepsDir
  $downloads = Join-Path $depsRoot "downloads"
  $obsSourceRoot = Join-Path $depsRoot "obs-source"
  $obsPortableRoot = Join-Path $depsRoot "obs-portable"
  $importLibDir = Join-Path $depsRoot "obs-import-lib"
  New-Item -ItemType Directory -Force -Path $downloads, $obsSourceRoot, $obsPortableRoot, $importLibDir | Out-Null

  $tag = $OBSVersion
  $portableZip = $null
  $sourceZip = $null

  if (-not $SkipDownload) {
    if ($OBSVersion -eq "latest") {
      Write-Host "Resolving latest OBS Studio release..."
      $release = Invoke-RestMethod -Uri "https://api.github.com/repos/obsproject/obs-studio/releases/latest" -Headers @{"User-Agent"="arsonkupik-build"}
      $tag = $release.tag_name
      $asset = $release.assets |
        Where-Object { $_.name -match 'Windows.*(x64|64).*\.zip$' -and $_.name -notmatch 'Installer|symbols|pdb' } |
        Select-Object -First 1
      if (-not $asset) {
        $asset = $release.assets |
          Where-Object { $_.name -match 'Windows.*\.zip$' -and $_.name -notmatch 'Installer|symbols|pdb' } |
          Select-Object -First 1
      }
      if (-not $asset) {
        throw "Could not find an OBS Windows portable zip asset in latest release $tag."
      }
      $portableZip = Join-Path $downloads $asset.name
      if (-not (Test-Path $portableZip)) {
        Write-Host "Downloading OBS portable: $($asset.name)"
        Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $portableZip
      }
    } else {
      if ($tag -notmatch '^v') { $tagForUrl = $tag } else { $tagForUrl = $tag }
      $guessName = "OBS-Studio-$tagForUrl-Windows-x64.zip"
      $portableZip = Join-Path $downloads $guessName
      if (-not (Test-Path $portableZip)) {
        Write-Host "Downloading OBS portable guess: $guessName"
        Invoke-WebRequest -Uri "https://github.com/obsproject/obs-studio/releases/download/$tagForUrl/$guessName" -OutFile $portableZip
      }
    }

    $sourceZip = Join-Path $downloads "obs-studio-$tag-source.zip"
    if (-not (Test-Path $sourceZip)) {
      Write-Host "Downloading OBS source tag: $tag"
      Invoke-WebRequest -Uri "https://github.com/obsproject/obs-studio/archive/refs/tags/$tag.zip" -OutFile $sourceZip
    }

    if (-not (Get-ChildItem $obsSourceRoot -Recurse -Filter obs-module.h -ErrorAction SilentlyContinue | Select-Object -First 1)) {
      Write-Host "Extracting OBS source..."
      Remove-Item -Recurse -Force $obsSourceRoot -ErrorAction SilentlyContinue
      New-Item -ItemType Directory -Force -Path $obsSourceRoot | Out-Null
      Expand-Archive -Path $sourceZip -DestinationPath $obsSourceRoot -Force
    }

    if (-not (Get-ChildItem $obsPortableRoot -Recurse -Filter obs.dll -ErrorAction SilentlyContinue | Select-Object -First 1)) {
      Write-Host "Extracting OBS portable..."
      Remove-Item -Recurse -Force $obsPortableRoot -ErrorAction SilentlyContinue
      New-Item -ItemType Directory -Force -Path $obsPortableRoot | Out-Null
      Expand-Archive -Path $portableZip -DestinationPath $obsPortableRoot -Force
    }
  }

  $obsSource = Get-ChildItem $obsSourceRoot -Directory | Where-Object { Test-Path (Join-Path $_.FullName "libobs\obs-module.h") } | Select-Object -First 1
  if (-not $obsSource) {
    $obsModule = Get-ChildItem $obsSourceRoot -Recurse -Filter obs-module.h -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($obsModule) { $obsSource = Get-Item (Split-Path -Parent (Split-Path -Parent $obsModule.FullName)) }
  }
  if (-not $obsSource) { throw "OBS source root with libobs/obs-module.h was not found under $obsSourceRoot." }

  $obsDll = Get-ChildItem $obsPortableRoot -Recurse -Filter obs.dll -ErrorAction SilentlyContinue | Where-Object { $_.FullName -match 'bin.*64bit|bin.*64-bit|bin' } | Select-Object -First 1
  if (-not $obsDll) { $obsDll = Get-ChildItem $obsPortableRoot -Recurse -Filter obs.dll -ErrorAction SilentlyContinue | Select-Object -First 1 }
  if (-not $obsDll) { throw "obs.dll was not found under $obsPortableRoot." }

  $defPath = Join-Path $importLibDir "obs.def"
  $libPath = Join-Path $importLibDir "obs.lib"

  Write-Host "Generating import library from $($obsDll.FullName)"
  $exports = & dumpbin /nologo /exports $obsDll.FullName
  $names = New-Object System.Collections.Generic.List[string]
  foreach ($line in $exports) {
    if ($line -match '^\s*\d+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+(.+?)\s*$') {
      $name = $Matches[1].Trim()
      if ($name -and $name -notmatch '^\?') { $names.Add($name) }
    }
  }
  if ($names.Count -eq 0) { throw "No C exports were parsed from obs.dll." }
  @("LIBRARY obs.dll", "EXPORTS") + ($names | Sort-Object -Unique | ForEach-Object { "  $_" }) | Set-Content -Encoding ASCII $defPath
  & lib /nologo /def:$defPath /machine:x64 /out:$libPath | Write-Host
  if ($LASTEXITCODE -ne 0) { throw "lib.exe failed to generate OBS import library." }
  if (-not (Test-Path $libPath)) { throw "Failed to generate $libPath." }

  Write-ObsConfigStub -ObsSourcePath $obsSource.FullName -Tag $tag

  Write-Host "Configuring ArSonKuPik OBS plugin..."
  & $CMakeExe -S . -B $BuildDir `
    -DBUILD_OBS_PLUGIN=ON `
    -DBUILD_STANDALONE_TESTS=ON `
    -DARSONKUPIK_OBS_SOURCE_DIR="$($obsSource.FullName)" `
    -DARSONKUPIK_OBS_IMPLIB="$libPath"
  if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

  Write-Host "Building..."
  & $CMakeExe --build $BuildDir --config $Config
  if ($LASTEXITCODE -ne 0) { throw "CMake build failed. No install/package step will run." }

  Write-Host "Installing to package/..."
  & $CMakeExe --install $BuildDir --config $Config --prefix "$root/package"
  if ($LASTEXITCODE -ne 0) { throw "CMake install failed." }

  Write-Host "Creating OBS ProgramData-ready package..."
  & "$root\scripts\package-windows-programdata.ps1" -BuildDir $BuildDir -Config $Config
  if ($LASTEXITCODE -ne 0) { throw "ProgramData packaging failed." }

  if ($InstallToProgramData) {
    & "$root\scripts\package-windows-programdata.ps1" -BuildDir $BuildDir -Config $Config -InstallToProgramData
    if ($LASTEXITCODE -ne 0) { throw "ProgramData install failed." }
  }

  Write-Host ""
  Write-Host "DONE. Ready-to-copy package:"
  Write-Host "  $root\package-programdata\arsonkupik-obs-audio-enhancer"
  Write-Host ""
  Write-Host "Zip it for distribution or copy this folder to:"
  Write-Host "  C:\ProgramData\obs-studio\plugins\"
} finally {
  Pop-Location
}
