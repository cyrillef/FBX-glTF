@echo off
%~d0
cd /d %~dp0

@echo Processing %1
call test-function %1 %1.fbx test

:end
pause
