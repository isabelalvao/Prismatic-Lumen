@echo off
setlocal
cd /d "%~dp0"
if not exist logs mkdir logs
echo A iniciar VR. Liga o Quest 3 pelo Virtual Desktop e seleciona VDXR no Streamer.
echo Registo: logs\vr.log
fantasynt.exe --vr %* > logs\vr.log 2>&1
set "result=%errorlevel%"
if not "%result%"=="0" (
  type logs\vr.log
  echo.
  echo Falha ao iniciar VR. Executa Diagnose-VR.cmd e consulta LE-ME.txt.
  pause
)
exit /b %result%
