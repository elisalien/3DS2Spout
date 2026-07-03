@echo off
title 3DS2SPOUT — Installation
cd /d "%~dp0.."
powershell -ExecutionPolicy Bypass -File "%~dp0install-all.ps1" %*
pause
