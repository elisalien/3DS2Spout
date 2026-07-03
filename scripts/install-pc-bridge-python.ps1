#Requires -Version 5.1
<#
.SYNOPSIS
  Installe les dependances Python du bridge (alternative a Rust).

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\scripts\install-pc-bridge-python.ps1
#>
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Bridge = Join-Path $Root "pc-bridge"

Write-Host "=== Dependances Python (pc-bridge) ===" -ForegroundColor Cyan

$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) {
    Write-Host "Python introuvable. Installe Python 3.10+ depuis python.org" -ForegroundColor Red
    exit 1
}

python -m pip install --upgrade pip
python -m pip install -r (Join-Path $Bridge "requirements.txt")

Write-Host ""
Write-Host "OK. Lance le bridge:" -ForegroundColor Green
Write-Host "  powershell -ExecutionPolicy Bypass -File .\scripts\run-pc-bridge.ps1"
Write-Host ""
Write-Host "Spout: copie SpoutLibrary.dll dans pc-bridge\vendor\"
Write-Host "  https://github.com/leadedge/Spout2/releases"
