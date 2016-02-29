@echo off
%~d0
cd /d %~dp0

for /d %%d in (*) do (
	@echo Processing %%d
	call test-function %%d %%d.fbx test
)

:end
pause
