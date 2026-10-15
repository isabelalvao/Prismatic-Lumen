@echo off
setlocal
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Verify-HandTracking.ps1" %*
set "result=%errorlevel%"
echo.
pause
exit /b %result%
