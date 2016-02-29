@echo off

if not exist %1\out mkdir %1\out
..\x64\Debug\glTF.exe -f %1\%2 -o %1\out -n %3 -c
