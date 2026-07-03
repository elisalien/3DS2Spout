#Requires -Version 5.1
<#
.SYNOPSIS
  Build 3DS2SPOUT release zip for GitHub (3dsx + install scripts + pc-bridge).

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
$RelDir = Join-Path $AppDir "release\3DS2SPOUT-$Version"
$Zip = Join-Path $AppDir "release\3DS2SPOUT-$Version.zip"

function Invoke-Make3dsx {
    param([string] $PcIp)
    $makeArgs = @("PC_IP=$PcIp")

    if (Get-Command make -ErrorAction SilentlyContinue) {
        Push-Location $AppDir
        try {
            & make clean 2>$null
            & make @makeArgs
            if ($LASTEXITCODE -ne 0) { throw "make failed ($LASTEXITCODE)" }
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
    $cmd = "export PATH=/opt/devkitpro/devkitARM/bin:/opt/devkitpro/tools/bin:/usr/bin:`$PATH; cd '$unix' && make clean && make PC_IP=$PcIp"
    & $bash -lc $cmd
    if ($LASTEXITCODE -ne 0) { throw "make failed via devkitPro bash" }
}

function Copy-Tree {
    param(
        [string] $Source,
        [string] $Destination,
        [string[]] $ExcludeDirNames = @(),
        [string[]] $ExcludeFilePatterns = @()
    )
    if (-not (Test-Path $Source)) {
        throw "Missing source path: $Source"
    }
    $null = New-Item -ItemType Directory -Force -Path $Destination
    Get-ChildItem -LiteralPath $Source -Force | ForEach-Object {
        if ($ExcludeDirNames -contains $_.Name) { return }
        foreach ($pat in $ExcludeFilePatterns) {
            if ($_.Name -like $pat) { return }
        }
        $destPath = Join-Path $Destination $_.Name
        if ($_.PSIsContainer) {
            Copy-Tree -Source $_.FullName -Destination $destPath -ExcludeDirNames $ExcludeDirNames -ExcludeFilePatterns $ExcludeFilePatterns
        } else {
            Copy-Item -LiteralPath $_.FullName -Destination $destPath -Force
        }
    }
}

function Invoke-StageRelease {
    param([string] $Ver, [string] $PcIp)

    $threeDsx = Join-Path $AppDir "3ds-cam-stream.3dsx"
    if (-not (Test-Path $threeDsx)) {
        throw "3ds-cam-stream.3dsx not found after build"
    }

    if (Test-Path $RelDir) { Remove-Item $RelDir -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $RelDir | Out-Null

    Copy-Item $threeDsx $RelDir -Force
    Copy-Item (Join-Path $AppDir "example.cfg") (Join-Path $RelDir "3ds-cam-stream.cfg.example") -Force
    Copy-Item (Join-Path $AppDir "release\README.txt") (Join-Path $RelDir "README.txt") -Force

    foreach ($file in @("README.md", "README.en.md", "LICENSE", "run-bridge.cmd", "install-buildtools.bat")) {
        Copy-Item (Join-Path $Root $file) (Join-Path $RelDir $file) -Force
    }

    Copy-Tree -Source (Join-Path $Root "install") -Destination (Join-Path $RelDir "install")
    Copy-Tree -Source (Join-Path $Root "docs") -Destination (Join-Path $RelDir "docs")
    Copy-Tree -Source (Join-Path $Root "resolume") -Destination (Join-Path $RelDir "resolume")
    Copy-Tree -Source (Join-Path $Root "pc-bridge") -Destination (Join-Path $RelDir "pc-bridge") `
        -ExcludeDirNames @("target", "__pycache__") -ExcludeFilePatterns @("*.dll", "*.pyc")

    if (Test-Path $Zip) { Remove-Item $Zip -Force }
    Compress-Archive -Path (Join-Path $RelDir "*") -DestinationPath $Zip -Force
}

Write-Host ""
Write-Host "=== 3DS2SPOUT release v$Version ===" -ForegroundColor Cyan
Write-Host ""

Invoke-Make3dsx -PcIp $ReleasePcIp
Invoke-StageRelease -Ver $Version -PcIp $ReleasePcIp

if (Test-Path $Zip) {
    $sizeKb = [math]::Round((Get-Item $Zip).Length / 1KB)
    Write-Host ""
    Write-Host "OK: $Zip ($sizeKb KB)" -ForegroundColor Green
    Write-Host "Upload this file to GitHub Releases." -ForegroundColor Yellow
} else {
    Write-Host "Zip not found: $Zip" -ForegroundColor Red
    exit 1
}
