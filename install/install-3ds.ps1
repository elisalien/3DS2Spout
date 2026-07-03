#Requires -Version 5.1
<#
.SYNOPSIS
  Assistant d'installation du homebrew sur la 3DS (WiFi + FTPD).
#>
param(
    [string] $ThreeDSIp
)

$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Target = Join-Path $Root "scripts\install-to-3ds.ps1"

Write-Host ""
Write-Host "=== Installation 3DS2SPOUT sur la 3DS ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Avant de continuer:" -ForegroundColor White
Write-Host "  1. Luma3DS + Homebrew Launcher installes"
Write-Host "  2. FTPD lance sur la 3DS (note l'IP affichee)"
Write-Host "  3. PC et 3DS sur le meme WiFi"
Write-Host "  4. devkitPro installe (make, 3dslink)"
Write-Host ""

if (-not $ThreeDSIp) {
    $ThreeDSIp = Read-Host "Adresse IP de la 3DS (ex. 192.168.1.45)"
}
if ([string]::IsNullOrWhiteSpace($ThreeDSIp)) {
    Write-Host "IP requise." -ForegroundColor Red
    exit 1
}

$pwd = Read-Host "Mot de passe FTP (Entree si vide / anonymous)"
$args = @("-ExecutionPolicy", "Bypass", "-File", $Target, "-ThreeDSIp", $ThreeDSIp.Trim())
if ($pwd) { $args += @("-FtpPassword", $pwd) }

& powershell @args
exit $LASTEXITCODE
