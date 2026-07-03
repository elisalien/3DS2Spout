#Requires -Version 5.1
<#
.SYNOPSIS
  Lance le PC bridge — Rust si disponible, sinon Python.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\scripts\run-pc-bridge.ps1
#>
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Bridge = Join-Path $Root "pc-bridge"
$CargoBin = Join-Path $env:USERPROFILE ".cargo\bin"

if (Test-Path $CargoBin) {
    $env:PATH = "$CargoBin;$env:PATH"
}

function Start-PythonBridge {
    $req = Join-Path $Bridge "requirements.txt"
    python -m pip show qoi 2>$null | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Installation dependances Python..." -ForegroundColor Yellow
        & (Join-Path $Root "scripts\install-pc-bridge-python.ps1")
    }
    Push-Location $Bridge
    try {
        Write-Host ""
        Write-Host "3DS2SPOUT bridge (Python) - UDP 5000" -ForegroundColor Green
        python receiver.py
    } finally {
        Pop-Location
    }
}

if (Get-Command cargo -ErrorAction SilentlyContinue) {
    Push-Location $Bridge
    try {
        if (-not (Test-Path "target\release\pc-bridge.exe")) {
            Write-Host "Build Rust (premiere fois)..." -ForegroundColor Yellow
            cargo build --release 2>&1
            if ($LASTEXITCODE -ne 0) {
                Write-Host "Build Rust echoue -> Python" -ForegroundColor Yellow
                Pop-Location
                Start-PythonBridge
                return
            }
        }
        Write-Host "3DS2SPOUT bridge (Rust) - UDP 5000" -ForegroundColor Green
        & ".\target\release\pc-bridge.exe"
    } catch {
        Pop-Location
        Start-PythonBridge
    } finally {
        if ((Get-Location).Path -eq $Bridge) { Pop-Location }
    }
} else {
    Write-Host "cargo absent - mode Python" -ForegroundColor Yellow
    Start-PythonBridge
}
