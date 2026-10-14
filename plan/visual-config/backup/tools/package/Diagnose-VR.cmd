@echo off
setlocal
cd /d "%~dp0"
if not exist logs mkdir logs
fantasynt.exe --xr-probe > logs\xr-probe.log 2>&1
set "result=%errorlevel%"
type logs\xr-probe.log
echo.
echo Registo guardado em logs\xr-probe.log
pause
exit /b %result%
