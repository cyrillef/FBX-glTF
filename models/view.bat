@echo off
%~d0
cd /d %~dp0

call "C:\Program Files\nodejs\nodevars.bat"

@echo View %1
del /q /f ..\lmv\models\*.* > nul
copy /y %1\out\*.* ..\lmv\models > nul
start chrome http://localhost:8080/ --aggressive-cache-discard
http-server ..\lmv

:end
pause
