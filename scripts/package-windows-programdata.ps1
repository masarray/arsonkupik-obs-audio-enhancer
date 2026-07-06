param(
  [string]$BuildDir = "build",
  [string]$Config = "Release",
  [switch]$InstallToProgramData
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$pluginName = "arsonkupik-obs-audio-enhancer"
$dllName = "$pluginName.dll"

Push-Location $root
try {
  $dllCandidates = @(
    (Join-Path $root "$BuildDir\$Config\$dllName"),
    (Join-Path $root "$BuildDir\$dllName"),
    (Join-Path $root "package\obs-plugins\64bit\$dllName")
  )

  $dllPath = $null
  foreach ($candidate in $dllCandidates) {
    if (Test-Path $candidate) {
      $dllPath = $candidate
      break
    }
  }

  if (-not $dllPath) {
    throw "Plugin DLL not found. Build first, then run this package script. Checked: $($dllCandidates -join '; ')"
  }

  $outRoot = Join-Path $root "package-programdata\$pluginName"
  $binOut = Join-Path $outRoot "bin\64bit"
  $localeOut = Join-Path $outRoot "data\locale"
  New-Item -ItemType Directory -Force -Path $binOut, $localeOut | Out-Null

  Copy-Item $dllPath (Join-Path $binOut $dllName) -Force
  Copy-Item (Join-Path $root "data\locale\en-US.ini") (Join-Path $localeOut "en-US.ini") -Force

  Write-Host "ProgramData-ready package created at: $outRoot"

  if ($InstallToProgramData) {
    $target = "C:\ProgramData\obs-studio\plugins\$pluginName"
    New-Item -ItemType Directory -Force -Path (Join-Path $target "bin\64bit"), (Join-Path $target "data\locale") | Out-Null
    Copy-Item (Join-Path $binOut $dllName) (Join-Path $target "bin\64bit\$dllName") -Force
    Copy-Item (Join-Path $localeOut "en-US.ini") (Join-Path $target "data\locale\en-US.ini") -Force
    Write-Host "Installed to: $target"
  }
} finally {
  Pop-Location
}
