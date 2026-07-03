#Requires -Version 5.1
<#
.SYNOPSIS
  Installe les dependances PC pour 3DS2SPOUT (Python + SpoutLibrary.dll auto).
  Rust est optionnel : -WithRust (le bridge Python suffit).
#>
[CmdletBinding()]
param(
    [switch] $WithRust,
    [switch] $Quiet
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Bridge = Join-Path $Root "pc-bridge"
$Vendor = Join-Path $Bridge "vendor"
$SpoutDll = Join-Path $Vendor "SpoutLibrary.dll"

# Release Spout2 epinglee (zip 3 Mo, empreinte verifiee)
$SpoutUrl = "https://github.com/leadedge/Spout2/releases/download/2.007.017/Spout-SDK-binaries_2-007-017_1.zip"
$SpoutSha256 = "695F20E3505FA0DA51B2EB959AF359F5D9E2C914BB9676E9118D19F6A5424BF4"

function Write-Step($Text) {
    if (-not $Quiet) { Write-Host $Text -ForegroundColor Cyan }
}

function Write-Ok($Text) {
    if (-not $Quiet) { Write-Host "  [OK] $Text" -ForegroundColor Green }
}

function Write-Warn($Text) {
    if (-not $Quiet) { Write-Host "  [!] $Text" -ForegroundColor Yellow }
}

function Write-Fail($Text) {
    if (-not $Quiet) { Write-Host "  [X] $Text" -ForegroundColor Red }
}

Write-Step ""
Write-Step "=== 3DS2SPOUT - Installation des dependances PC ==="
Write-Step ""

# --- Python ---
Write-Step "1/3 Python 3.10+"
$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) {
    Write-Fail "Python introuvable."
    Write-Warn "Telecharge: https://www.python.org/downloads/"
    Write-Warn "Coche 'Add python.exe to PATH' lors de l'installation."
    exit 1
}
Write-Ok ("Python: " + (& python --version 2>&1))

# --- Packages ---
Write-Step "2/3 Packages Python (pc-bridge)"
python -m pip install --upgrade pip --quiet
python -m pip install -r (Join-Path $Bridge "requirements.txt")
if ($LASTEXITCODE -ne 0) { throw "pip install a echoue" }
Write-Ok "qoi, python-osc installes"

# --- Spout DLL (auto) ---
Write-Step "3/3 SpoutLibrary.dll"
if (Test-Path $SpoutDll) {
    Write-Ok "SpoutLibrary.dll deja presente dans pc-bridge\vendor\"
} else {
    if (-not (Test-Path $Vendor)) {
        New-Item -ItemType Directory -Path $Vendor -Force | Out-Null
    }
    $tmpZip = Join-Path $env:TEMP "spout-sdk-binaries.zip"
    $tmpDir = Join-Path $env:TEMP "spout-sdk-binaries"
    try {
        Write-Step "  Telechargement de Spout 2.007.017 (3 Mo)..."
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
        Invoke-WebRequest -Uri $SpoutUrl -OutFile $tmpZip -UseBasicParsing
        $hash = (Get-FileHash $tmpZip -Algorithm SHA256).Hash
        if ($hash -ne $SpoutSha256) { throw "empreinte SHA256 inattendue" }
        if (Test-Path $tmpDir) { Remove-Item $tmpDir -Recurse -Force }
        Expand-Archive -Path $tmpZip -DestinationPath $tmpDir -Force
        $dll = Get-ChildItem $tmpDir -Recurse -Filter "SpoutLibrary.dll" |
            Where-Object { $_.FullName -match "x64" } | Select-Object -First 1
        if (-not $dll) {
            $dll = Get-ChildItem $tmpDir -Recurse -Filter "SpoutLibrary.dll" | Select-Object -First 1
        }
        if (-not $dll) { throw "SpoutLibrary.dll absente du zip" }
        Copy-Item $dll.FullName $SpoutDll -Force
        Write-Ok "SpoutLibrary.dll installee automatiquement"
    } catch {
        Write-Warn "Telechargement automatique impossible : $($_.Exception.Message)"
        Write-Warn "Manuel : https://github.com/leadedge/Spout2/releases"
        Write-Warn "  -> zip 'Spout-SDK-binaries...' -> copier SpoutLibrary.dll (x64)"
        Write-Warn "  -> dans : $Vendor"
    } finally {
        if (Test-Path $tmpZip) { Remove-Item $tmpZip -Force -ErrorAction SilentlyContinue }
        if (Test-Path $tmpDir) { Remove-Item $tmpDir -Recurse -Force -ErrorAction SilentlyContinue }
    }
}

# --- Rust (opt-in) ---
if ($WithRust) {
    Write-Step "Option : Rust / cargo (bridge plus rapide)"
    if (Get-Command cargo -ErrorAction SilentlyContinue) {
        Write-Ok ("cargo: " + (& cargo --version 2>&1))
    } else {
        $rustScript = Join-Path $Root "scripts\install-rust.ps1"
        if (Test-Path $rustScript) {
            Write-Warn "cargo absent - tentative d'installation via winget..."
            & $rustScript
        } else {
            Write-Warn "cargo absent. Le bridge Python sera utilise."
        }
    }
}

Write-Step ""
Write-Ok "Dependances PC installees."
Write-Step ""
Write-Step "Prochaine etape : install\run-bridge.cmd"
Write-Step ""
