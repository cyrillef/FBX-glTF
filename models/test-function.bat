@echo off

if not exist %1\out mkdir %1\out > nul
del /q /f %1\out\*.* > nul
..\x64\Debug\glTF.exe -f %1\%2 -o %1\out -n %3 -c > %1\out\%1.txt

diff -b -w -B -r %1\out %1\test > %1.diff
if errorlevel 1 echo   %1 test failed
echo.