@echo off
title 3DS2SPOUT — Installation 3DS
cd /d "%~dp0.."
powershell -ExecutionPolicy Bypass -File "%~dp0install-3ds.ps1" %*
pause
