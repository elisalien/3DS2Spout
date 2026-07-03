@echo off
title 3DS2SPOUT — Release zip
cd /d "%~dp0.."
set VER=%~1
if "%VER%"=="" set VER=1.0.0
powershell -ExecutionPolicy Bypass -File "%~dp0make-release.ps1" -Version %VER%
pause
