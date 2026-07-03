@echo off
title 3DS2SPOUT — Prerequis
cd /d "%~dp0.."
powershell -ExecutionPolicy Bypass -File "%~dp0check-prerequisites.ps1"
pause
