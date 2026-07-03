#Requires -Version 5.1
<#
.SYNOPSIS
  Installe Rust (rustup + cargo) sur Windows.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\scripts\install-rust.ps1
#>
$ErrorActionPreference = "Stop"

Write-Host "=== Installation Rust (cargo) ===" -ForegroundColor Cyan

if (Get-Command cargo -ErrorAction SilentlyContinue) {
    Write-Host "cargo deja installe:" -ForegroundColor Green
    cargo --version
    rustc --version
    Write-Host ""
    Write-Host "Si cargo build echoue (link.exe), utilise le bridge Python:"
    Write-Host "  .\scripts\run-pc-bridge.ps1"
    exit 0
}

if (Get-Command winget -ErrorAction SilentlyContinue) {
    Write-Host "Installation via winget (Rustlang.Rustup)..." -ForegroundColor Yellow
    winget install Rustlang.Rustup --accept-package-agreements --accept-source-agreements
} else {
    Write-Host "winget introuvable. Telecharge rustup:" -ForegroundColor Yellow
    Write-Host "  https://rustup.rs"
    Write-Host "  ou: https://win.rustup.rs/x86_64"
    exit 1
}

$cargoPath = Join-Path $env:USERPROFILE ".cargo\bin\cargo.exe"
if (Test-Path $cargoPath) {
    $env:PATH = "$(Split-Path $cargoPath -Parent);$env:PATH"
}

if (Get-Command cargo -ErrorAction SilentlyContinue) {
    Write-Host ""
    Write-Host "OK:" -ForegroundColor Green
    cargo --version
    rustc --version
    Write-Host ""
    Write-Host "Ferme et rouvre le terminal, puis:"
    Write-Host "  .\scripts\run-pc-bridge.ps1"
    Write-Host ""
    Write-Host "Ou sans Rust (Python deja installe):"
    Write-Host "  .\scripts\install-pc-bridge-python.ps1"
    Write-Host "  python pc-bridge\receiver.py"
} else {
    Write-Host ""
    Write-Host "Rust installe. FERME ce terminal, ouvre-en un NOUVEAU, puis:" -ForegroundColor Yellow
    Write-Host "  .\scripts\run-pc-bridge.ps1"
}
