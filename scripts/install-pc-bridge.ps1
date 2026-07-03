#Requires -Version 5.1
<#
.SYNOPSIS
  Build the PC bridge and optionally fetch SpoutLibrary.dll.

.EXAMPLE
  .\scripts\install-pc-bridge.ps1
  .\scripts\install-pc-bridge.ps1 -SkipSpoutDownload
#>
[CmdletBinding()]
param(
    [switch] $SkipSpoutDownload,
    [switch] $Release
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$BridgeDir = Join-Path $Root "pc-bridge"
$VendorDir = Join-Path $BridgeDir "vendor"
$DllPath = Join-Path $VendorDir "SpoutLibrary.dll"

Write-Host "=== 3DS2SPOUT — PC bridge setup ===" -ForegroundColor Cyan

if (-not (Get-Command cargo -ErrorAction SilentlyContinue)) {
    Write-Host "ERROR: Rust/cargo not found. Install from https://rustup.rs" -ForegroundColor Red
    exit 1
}

Push-Location $BridgeDir
try {
    if ($Release) {
        cargo build --release
        $Bin = Join-Path $BridgeDir "target\release\pc-bridge.exe"
    } else {
        cargo build
        $Bin = Join-Path $BridgeDir "target\debug\pc-bridge.exe"
    }
} finally {
    Pop-Location
}

if (-not $SkipSpoutDownload -and -not (Test-Path $DllPath)) {
    Write-Host ""
    Write-Host "SpoutLibrary.dll not found in vendor/." -ForegroundColor Yellow
    Write-Host "Download manually from:" -ForegroundColor Yellow
    Write-Host "  https://github.com/leadedge/Spout2/releases"
    Write-Host "  → copy SpoutLibrary.dll to: $VendorDir"
    Write-Host ""
    Write-Host "Or place it next to: $Bin"
}

Write-Host ""
Write-Host "Run:" -ForegroundColor Green
Write-Host "  & '$Bin'"
Write-Host ""
