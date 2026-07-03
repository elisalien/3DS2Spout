@echo off
cd /d "%~dp0.."
powershell -ExecutionPolicy Bypass -File "%~dp0run-pc-bridge.ps1"
pause
