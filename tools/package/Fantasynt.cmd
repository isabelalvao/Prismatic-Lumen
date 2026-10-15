@echo off
cd /d "%~dp0"
fantasynt.exe %*
exit /b %errorlevel%
