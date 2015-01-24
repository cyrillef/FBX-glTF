@echo off
%~d0
cd /d %~dp0..

set GLTF_EXE=x64\Debug\glTF.exe

if not exist three.js\models\duck\nul (mkdir three.js\models\duck) else (del /q three.js\models\duck\*.*)
%GLTF_EXE% -f models\duck\duck.fbx -o three.js\models\duck -n duck -c

rem if not exist three.js\models\au\nul (mkdir three.js\models\au) else (del /q three.js\models\au\*.*)
rem %GLTF_EXE% -f models\au\au.fbx -o three.js\models\au -c
rem %GLTF_EXE% -f models\au\au.fbx -o three.js\models\au -n au-embbed -e
rem %GLTF_EXE% -f models\au\au3.fbx -o three.js\models\au -c

if not exist three.js\models\monster\nul (mkdir three.js\models\monster) else (del /q three.js\models\monster\*.*)
%GLTF_EXE% -f models\monster\monster-animated-character.fbx -o three.js\models\monster -n monster -c

if not exist three.js\models\rambler\nul (mkdir three.js\models\rambler) else (del /q three.js\models\rambler\*.*)
%GLTF_EXE% -f models\rambler\Rambler.fbx -o three.js\models\rambler -n rambler -c

if not exist three.js\models\rambler\nul (mkdir three.js\models\rambler) else (del /q three.js\models\rambler\*.*)
%GLTF_EXE% -f models\rambler\Rambler-0.fbx -o three.js\models\rambler -n rambler0 -c

rem if not exist three.js\models\SuperMurdoch\nul (mkdir three.js\models\SuperMurdoch) else (del /q three.js\models\SuperMurdoch\*.*)
rem %GLTF_EXE% -f models\SuperMurdoch\SuperMurdoch.fbx -o three.js\models\SuperMurdoch -n SuperMurdoch -c

rem if not exist three.js\models\teapot\nul (mkdir three.js\models\teapot) else (del /q three.js\models\teapot\*.*)
rem %GLTF_EXE% -f models\teapot\teapot.fbx -o three.js\models\teapot -n teapot -c

if not exist three.js\models\wine\nul (mkdir three.js\models\wine) else (del /q three.js\models\wine\*.*)
%GLTF_EXE% -f models\wine\wine.fbx -o three.js\models\wine -n wine -c

pause
