#Requires -Version 5.1
<#
.SYNOPSIS
  Build 3DS2SPOUT release zip for GitHub (3dsx + cfg example + README).

.EXAMPLE
  .\scripts\make-release.ps1
  .\scripts\make-release.ps1 -Version 1.0.1
#>
[CmdletBinding()]
param(
    [string] $Version = "1.0.0",
    [string] $ReleasePcIp = "192.168.0.1"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$AppDir = Join-Path $Root "3ds-cam-stream"

function Invoke-MakeRelease {
    param([string] $Ver, [string] $PcIp)
    $makeArgs = @("release", "VERSION=$Ver", "RELEASE_PC_IP=$PcIp")

    if (Get-Command make -ErrorAction SilentlyContinue) {
        Push-Location $AppDir
        try {
            & make @makeArgs
            if ($LASTEXITCODE -ne 0) { throw "make release failed ($LASTEXITCODE)" }
        } finally { Pop-Location }
        return
    }

    $bash = @(
        "${env:DEVKITPRO}\msys2\usr\bin\bash.exe",
        "C:\devkitPro\msys2\usr\bin\bash.exe"
    ) | Where-Object { Test-Path $_ } | Select-Object -First 1

    if (-not $bash) {
        throw "make not found. Open MSYS2 devkitPro or install devkitARM."
    }

    $unix = ($AppDir -replace '\\', '/')
    $cmd = "export PATH=/opt/devkitpro/devkitARM/bin:/opt/devkitpro/tools/bin:/usr/bin:`$PATH; cd '$unix' && make release VERSION=$Ver RELEASE_PC_IP=$PcIp"
    & $bash -lc $cmd
    if ($LASTEXITCODE -ne 0) { throw "make release failed via devkitPro bash" }
}

Write-Host ""
Write-Host "=== 3DS2SPOUT release v$Version ===" -ForegroundColor Cyan
Write-Host ""

Invoke-MakeRelease -Ver $Version -PcIp $ReleasePcIp

$zip = Join-Path $AppDir "release\3DS2SPOUT-$Version.zip"
if (Test-Path $zip) {
    Write-Host ""
    Write-Host "OK: $zip" -ForegroundColor Green
    Write-Host "Upload this file to GitHub Releases." -ForegroundColor Yellow
} else {
    Write-Host "Zip not found: $zip" -ForegroundColor Red
    exit 1
}
