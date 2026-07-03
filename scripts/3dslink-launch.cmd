@echo off
setlocal
cd /d "%~dp0.."

set "THREEDS_IP=10.129.63.64"
if not "%~1"=="" set "THREEDS_IP=%~1"

set "THREEDSX=%~dp0..\3ds-cam-stream\3ds-cam-stream.3dsx"
if not exist "%THREEDSX%" (
    echo Fichier introuvable: %THREEDSX%
    echo Compile d'abord dans MSYS: cd /d/CREA/TOOLS/3DS/apps/3ds-cam-stream ^&^& make
    exit /b 1
)

set "LINK="
if defined DEVKITPRO set "LINK=%DEVKITPRO%\tools\bin\3dslink.exe"
if not exist "%LINK%" set "LINK=C:\devkitPro\tools\bin\3dslink.exe"
if not exist "%LINK%" (
    echo 3dslink introuvable. Installe devkitPro ou ouvre MSYS2 devkitPro.
    exit /b 1
)

echo Sur la 3DS: Homebrew Launcher ouvert, appuie sur Y ^(NetLoader^)
echo Envoi vers %THREEDS_IP% ...
"%LINK%" -a %THREEDS_IP% "%THREEDSX%"
exit /b %ERRORLEVEL%
