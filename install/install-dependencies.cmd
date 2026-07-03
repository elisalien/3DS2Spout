@echo off
title 3DS2SPOUT — Dependances
cd /d "%~dp0.."
powershell -ExecutionPolicy Bypass -File "%~dp0install-dependencies.ps1" %*
pause
