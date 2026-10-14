@echo off
setlocal
cd /d "%~dp0"
if not exist logs mkdir logs
fantasynt.exe %* > logs\desktop.log 2>&1
set "result=%errorlevel%"
if not "%result%"=="0" (
  type logs\desktop.log
  pause
)
exit /b %result%
