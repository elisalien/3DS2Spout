#Requires -Version 5.1
<#
.SYNOPSIS
  Installation guidee 3DS2SPOUT - dependances PC + verification.
  Tout est automatique (Python packages + SpoutLibrary.dll).
#>
[CmdletBinding()]
param(
    [switch] $WithRust,
    [string] $ThreeDSIp
)

$ErrorActionPreference = "Stop"
$InstallDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $InstallDir
$GuidePdf = Join-Path $InstallDir "guide\GUIDE-DEBUTANT.pdf"

Write-Host ""
Write-Host "  +======================================+" -ForegroundColor DarkCyan
Write-Host "  |     3DS2SPOUT - Installation         |" -ForegroundColor DarkCyan
Write-Host "  +======================================+" -ForegroundColor DarkCyan
Write-Host ""

# 1. Dependencies (Python + Spout DLL auto)
Write-Host "[1/2] Dependances PC..." -ForegroundColor Cyan
$depArgs = @("-ExecutionPolicy", "Bypass", "-File", (Join-Path $InstallDir "install-dependencies.ps1"))
if ($WithRust) { $depArgs += "-WithRust" }
& powershell @depArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# 2. Prerequisites check
Write-Host "[2/2] Verification des prerequis..." -ForegroundColor Cyan
& powershell -ExecutionPolicy Bypass -File (Join-Path $InstallDir "check-prerequisites.ps1")

# Optional 3DS deploy (dev, WiFi + FTPD)
if ($ThreeDSIp) {
    $script3ds = Join-Path $Root "scripts\install-to-3ds.ps1"
    if (Test-Path $script3ds) {
        & powershell -ExecutionPolicy Bypass -File $script3ds -ThreeDSIp $ThreeDSIp
    } else {
        Write-Host "  Script 3DS introuvable: $script3ds" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor DarkCyan
Write-Host "  Installation PC terminee." -ForegroundColor Green
Write-Host ""
Write-Host "  Il reste 2 choses :" -ForegroundColor White
Write-Host "    1. Copie 3ds-cam-stream.3dsx sur la carte SD (dossier /3ds/)" -ForegroundColor Yellow
Write-Host "    2. Lance install\run-bridge.cmd et note l'IP affichee" -ForegroundColor Yellow
Write-Host ""
if (Test-Path $GuidePdf) {
    Write-Host "  Guide debutant (PDF):" -ForegroundColor White
    Write-Host "    $GuidePdf" -ForegroundColor Yellow
    $open = Read-Host "  Ouvrir le PDF maintenant ? (O/n)"
    if ($open -ne "n" -and $open -ne "N") {
        Start-Process $GuidePdf
    }
}
Write-Host "========================================" -ForegroundColor DarkCyan
Write-Host ""
