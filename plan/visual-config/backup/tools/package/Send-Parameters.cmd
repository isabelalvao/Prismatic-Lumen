@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Send-Parameters.ps1" %*
exit /b %errorlevel%
