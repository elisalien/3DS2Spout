#Requires -Version 5.1
<#
.SYNOPSIS
  Verifie les prerequis 3DS2SPOUT et affiche un rapport.
#>
$ErrorActionPreference = "SilentlyContinue"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Bridge = Join-Path $Root "pc-bridge"
$SpoutDll = Join-Path $Bridge "vendor\SpoutLibrary.dll"

function Test-Cmd($Name) { $null -ne (Get-Command $Name -ErrorAction SilentlyContinue) }

$checks = @(
    @{
        Label = "Python 3.10+"
        Ok    = (Test-Cmd "python")
        Hint  = "https://www.python.org/downloads/"
    },
    @{
        Label = "pip + qoi (Python bridge)"
        Ok    = ((Test-Cmd "python") -and ((python -m pip show qoi 2>$null) -match "Name"))
        Hint  = "Lance install\install-dependencies.cmd"
    },
    @{
        Label = "Rust / cargo (optionnel)"
        Ok    = (Test-Cmd "cargo")
        Hint  = "Optionnel - Python suffit"
    },
    @{
        Label = "SpoutLibrary.dll"
        Ok    = (Test-Path $SpoutDll)
        Hint  = "https://github.com/leadedge/Spout2/releases -> pc-bridge\vendor\"
    },
    @{
        Label = "devkitPro / make (compilation 3DS)"
        Ok    = ((Test-Cmd "make") -or (Test-Path "C:\devkitPro\msys2\usr\bin\bash.exe"))
        Hint  = "https://devkitpro.org/wiki/Getting_Started"
    },
    @{
        Label = "3dslink (deploiement WiFi 3DS)"
        Ok    = ((Test-Cmd "3dslink") -or (Test-Path "C:\devkitPro\tools\bin\3dslink.exe"))
        Hint  = "pacman -S 3dslink (devkitPro MSYS2)"
    }
)

Write-Host ""
Write-Host "=== Prerequis 3DS2SPOUT ===" -ForegroundColor Cyan
Write-Host ""

$required = @("Python 3.10+", "pip + qoi (Python bridge)", "SpoutLibrary.dll")
$missingRequired = @()

foreach ($c in $checks) {
    $icon = if ($c.Ok) { "[OK]" } else { "[--]" }
    $color = if ($c.Ok) { "Green" } else { "Yellow" }
    Write-Host "  $icon $($c.Label)" -ForegroundColor $color
    if (-not $c.Ok) {
        Write-Host "       -> $($c.Hint)" -ForegroundColor DarkGray
        if ($required -contains $c.Label) { $missingRequired += $c.Label }
    }
}

Write-Host ""
if ($missingRequired.Count -eq 0) {
    Write-Host "Pret pour le bridge PC." -ForegroundColor Green
} else {
    Write-Host "Manquant (obligatoire PC): $($missingRequired -join ', ')" -ForegroundColor Yellow
    Write-Host "Lance: install\install-dependencies.cmd" -ForegroundColor Yellow
}
Write-Host ""
Write-Host "Guide complet: install\guide\GUIDE-DEBUTANT.pdf" -ForegroundColor Cyan
Write-Host ""
