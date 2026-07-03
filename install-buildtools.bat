@echo off
setlocal enabledelayedexpansion
title Installation Windows SDK + outils C++ (VS Build Tools 2019)

REM === Auto-elevation en administrateur ===
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Demande des droits administrateur...
    powershell -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

echo ============================================================
echo   Ajout du Windows SDK et du toolchain C++ x64/x86
echo   a Visual Studio Build Tools 2019
echo ============================================================
echo.

REM === Localisation du Visual Studio Installer ===
set "INSTALLER=C:\Program Files (x86)\Microsoft Visual Studio\Installer\setup.exe"
set "VSPATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools"

if not exist "%INSTALLER%" (
    echo [ERREUR] Visual Studio Installer introuvable :
    echo    "%INSTALLER%"
    echo.
    echo Ouvre "Visual Studio Installer" manuellement, ou reinstalle
    echo les Build Tools depuis https://aka.ms/vs/17/release/vs_BuildTools.exe
    echo.
    pause
    exit /b 1
)

if not exist "%VSPATH%" (
    echo [ERREUR] Build Tools 2019 introuvables :
    echo    "%VSPATH%"
    echo.
    pause
    exit /b 1
)

echo Lancement de l'installation...
echo.
echo   =====================================================
echo   Une fenetre de progression Microsoft va s'ouvrir.
echo   ATTENDS qu'elle affiche 100%% / "Termine" avant de
echo   continuer. Elle peut mettre plusieurs minutes.
echo   =====================================================
echo.

"%INSTALLER%" modify ^
    --installPath "%VSPATH%" ^
    --add Microsoft.VisualStudio.Workload.VCTools ^
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
    --includeRecommended ^
    --passive --norestart

set "RC=%errorlevel%"
echo.
if not "%RC%"=="0" if not "%RC%"=="3010" (
    echo ============================================================
    echo   [ATTENTION] Le lanceur a renvoye le code %RC%.
    echo ============================================================
    echo Si aucune fenetre de progression ne s'est ouverte, ouvre
    echo "Visual Studio Installer" a la main et coche la charge de
    echo travail "Developpement Desktop en C++".
    echo.
    pause
    exit /b %RC%
)

echo ============================================================
echo   Installation lancee.
echo ============================================================
echo.
echo IMPORTANT : attends la fin dans la fenetre Microsoft
echo (barre a 100%% / bouton "Fermer"), PUIS :
echo.
echo   1. FERME ce terminal et l'ancien.
echo   2. Menu Demarrer : lance
echo      "x64 Native Tools Command Prompt for VS 2019"
echo   3. Dans ce nouveau prompt :
echo         cd /d D:\CREA\TOOLS\3DS\apps\pc-bridge
echo         echo %%LIB%%
echo         cargo build --release
echo.
echo   ( echo %%LIB%% doit maintenant montrer 3 chemins,
echo     dont ...\Windows Kits\10\Lib\...\um\x64 )
echo.
pause
