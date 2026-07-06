param(
  [string]$BuildDir = "build",
  [string]$Config = "Release",
  [string]$Libobs_DIR = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
  $args = @("-S", ".", "-B", $BuildDir, "-DBUILD_OBS_PLUGIN=ON", "-DBUILD_STANDALONE_TESTS=ON")
  if ($Libobs_DIR -ne "") { $args += "-Dlibobs_DIR=$Libobs_DIR" }
  cmake @args
  cmake --build $BuildDir --config $Config
  cmake --install $BuildDir --config $Config --prefix "$root/package"
  Write-Host "Built package at: $root/package"
} finally {
  Pop-Location
}
