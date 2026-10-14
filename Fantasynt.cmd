@echo off
cd /d "%~dp0"
if not exist "build\Release\fantasynt.exe" (
    echo Compila primeiro com tools\build.cmd
    exit /b 1
)
start "" "%~dp0build\Release\fantasynt.exe" --config "%~dp0visual-config.json" %*
