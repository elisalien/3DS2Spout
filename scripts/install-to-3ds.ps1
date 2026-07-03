#Requires -Version 5.1
<#
.SYNOPSIS
  Build and install 3DS2SPOUT on the 3DS SD card over WiFi (no SD reader needed).
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $ThreeDSIp,

    [string] $PcIp,
    [int] $PcPort = 5000,
    [int] $FtpPort = 5000,
    [int] $ThreeDSLinkPort = 0,
    [string] $FtpUser = "anonymous",
    [string] $FtpPassword = "",
    [switch] $SkipBuild,
    [switch] $SkipFtp,
    [switch] $SkipLaunch
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$AppDir = Join-Path $Root "3ds-cam-stream"
$ThreeDsx = Join-Path $AppDir "3ds-cam-stream.3dsx"
$CfgLocal = Join-Path $env:TEMP "3ds-cam-stream.cfg"

function Get-LanIPv4 {
    if ($PcIp) { return $PcIp }
    $cfg = Get-NetIPConfiguration -ErrorAction SilentlyContinue |
        Where-Object { $_.IPv4DefaultGateway -and $_.NetAdapter.Status -eq "Up" } |
        Select-Object -First 1
    if ($cfg -and $cfg.IPv4Address) {
        return $cfg.IPv4Address.IPAddress
    }
    throw "Could not detect PC LAN IP. Pass -PcIp explicitly."
}

function Test-Command($Name) {
    $null -ne (Get-Command $Name -ErrorAction SilentlyContinue)
}

function Invoke-Make {
    param([string] $PcIpAddr, [int] $Port)
    if (Test-Command "make") {
        Push-Location $AppDir
        try {
            & make clean 2>$null
            & make "PC_IP=$PcIpAddr" "PC_PORT=$Port"
            if ($LASTEXITCODE -ne 0) { throw "make failed with exit code $LASTEXITCODE" }
        } finally { Pop-Location }
        return
    }
    $bash = @(
        "${env:DEVKITPRO}\msys2\usr\bin\bash.exe",
        "C:\devkitPro\msys2\usr\bin\bash.exe"
    ) | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $bash) {
        throw "make not found. Open MSYS2 devkitPro or set DEVKITPRO."
    }
    $cmd = "export PATH=/opt/devkitpro/devkitARM/bin:/opt/devkitpro/tools/bin:/usr/bin:`$PATH; cd '$($AppDir -replace '\\','/')' && make clean 2>/dev/null; make PC_IP=$PcIpAddr PC_PORT=$Port"
    & $bash -lc $cmd
    if ($LASTEXITCODE -ne 0) { throw "make failed via devkitPro bash" }
}

function Invoke-3dslink {
    param([string] $Ip, [string] $File)
    $args = @("-a", $Ip, $File)

    if (Test-Command "3dslink") {
        & 3dslink @args
        return $LASTEXITCODE
    }
    $link = @(
        "${env:DEVKITPRO}\tools\bin\3dslink.exe",
        "C:\devkitPro\tools\bin\3dslink.exe"
    ) | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ($link) {
        & $link @args
        return $LASTEXITCODE
    }
    return 1
}

function Invoke-FtpUpload {
    param(
        [string] $LocalPath,
        [string] $RemotePath
    )
    $uri = "ftp://${ThreeDSIp}:${FtpPort}/${RemotePath.TrimStart('/')}"
    Write-Host "  FTP upload: $RemotePath"
    $request = [System.Net.FtpWebRequest]::Create($uri)
    $request.Method = [System.Net.WebRequestMethods+Ftp]::UploadFile
    $request.UseBinary = $true
    $request.UsePassive = $true
    $request.KeepAlive = $false
    $request.Credentials = New-Object System.Net.NetworkCredential($FtpUser, $FtpPassword)
    $bytes = [System.IO.File]::ReadAllBytes($LocalPath)
    $request.ContentLength = $bytes.Length
    $stream = $request.GetRequestStream()
    $stream.Write($bytes, 0, $bytes.Length)
    $stream.Close()
    $response = $request.GetResponse()
    $response.Close()
}

function Ensure-FtpDirectory {
    param([string] $RemoteDir)
    $uri = "ftp://${ThreeDSIp}:${FtpPort}/${RemoteDir.TrimStart('/')}"
    try {
        $request = [System.Net.FtpWebRequest]::Create($uri)
        $request.Method = [System.Net.WebRequestMethods+Ftp]::MakeDirectory
        $request.Credentials = New-Object System.Net.NetworkCredential($FtpUser, $FtpPassword)
        $response = $request.GetResponse()
        $response.Close()
    } catch {
        # Directory may already exist
    }
}

Write-Host ""
Write-Host "=== 3DS2SPOUT - install SD via WiFi ===" -ForegroundColor Cyan
Write-Host ""

$detectedPcIp = Get-LanIPv4
Write-Host "PC IP (stream target): ${detectedPcIp}:$PcPort"
Write-Host "3DS IP (FTP): ${ThreeDSIp}:$FtpPort"

if (-not $SkipBuild) {
    Write-Host ""
    Write-Host "Building homebrew (PC_IP=$detectedPcIp)..." -ForegroundColor Yellow
    try {
        Invoke-Make -PcIpAddr $detectedPcIp -Port $PcPort
    } catch {
        Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
        exit 1
    }
}

if (-not (Test-Path $ThreeDsx)) {
    Write-Host "ERROR: Missing $ThreeDsx" -ForegroundColor Red
    exit 1
}
Write-Host "Built: $ThreeDsx"

@"
pc_ip=$detectedPcIp
pc_port=$PcPort
"@ | Set-Content -Path $CfgLocal -Encoding ASCII

if (-not $SkipFtp) {
    Write-Host ""
    Write-Host "Uploading to 3DS FTP..." -ForegroundColor Yellow
    try {
        Ensure-FtpDirectory -RemoteDir "3ds"
        Invoke-FtpUpload -LocalPath $ThreeDsx -RemotePath "3ds/3ds-cam-stream.3dsx"
        Invoke-FtpUpload -LocalPath $CfgLocal -RemotePath "3ds-cam-stream.cfg"
        Write-Host "  SD install OK:" -ForegroundColor Green
        Write-Host "    sdmc:/3ds/3ds-cam-stream.3dsx"
        Write-Host "    sdmc:/3ds-cam-stream.cfg"
    } catch {
        Write-Host "FTP upload failed: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "Tips: FTPD running on 3DS? Try -FtpPassword 1234 or -SkipFtp"
        if ($SkipLaunch) { exit 1 }
    }
}

if (-not $SkipLaunch) {
    Write-Host ""
    Write-Host "Launching via 3dslink (Homebrew Launcher open)..." -ForegroundColor Yellow
    $code = Invoke-3dslink -Ip $ThreeDSIp -File $ThreeDsx
    if ($code -ne 0) {
        Write-Host "3dslink failed - open sdmc:/3ds/3ds-cam-stream.3dsx manually" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "Done. Next on PC:" -ForegroundColor Green
Write-Host "  cd pc-bridge"
Write-Host "  cargo run --release"
Write-Host "  Resolume: add Spout source 3DS2SPOUT"
Write-Host ""
