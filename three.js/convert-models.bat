@echo off
%~d0
cd /d %~dp0..

set GLTF_EXE=x64\Debug\glTF.exe

if not exist three.js\models\duck\nul (mkdir three.js\models\duck) else (del /q three.js\models\duck\*.*)
%GLTF_EXE% -f models\duck\duck.fbx -o three.js\models\duck -n duck -c

pause


rem Duck
rem inFile =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2012\\Projects\\glTF\\duck.dae") ;
rem utility::string_t outDir2 =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\montagejs-glTF\\model\\duck-cyr\\") ;
rem outDir/*3*/ =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\three.js\\models\\gltf\\duck-cyr\\") ;

rem Rambler
rem utility::string_t inFiledae =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2012\\Projects\\glTF\\Rambler.dae") ;
rem inFile =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\three.js\\models\\gltf\\rambler\\Rambler-1.fbx") ;
rem outDir/*3*/ =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\three.js\\models\\gltf\\rambler\\") ;

rem SuperMurdoch
rem utility::string_t inFiledae =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2012\\Projects\\glTF\\SuperMurdoch.dae") ;
rem inFile =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\three.js\\models\\gltf\\SuperMurdoch\\SuperMurdoch.fbx") ;
rem outDir/*3*/ =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\three.js\\models\\gltf\\SuperMurdoch\\") ;

rem Wine
rem inFile =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2012\\Projects\\glTF\\wine.dae") ;

rem square
rem inFile =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\montagejs-glTF\\model\\duck-cyr\\cyr-square.dae") ;
rem utility::string_t squarefbx =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\montagejs-glTF\\model\\duck-cyr\\cyr-square.fbx") ;
rem outDir =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\montagejs-glTF\\model\\duck-cyr\\cyr-square.fbx") ;

rem AU
rem inFile =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\montagejs-glTF\\model\\au\\au.dae") ;
rem utility::string_t aufbx =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\montagejs-glTF\\model\\au\\au.fbx") ;
rem outDir =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\montagejs-glTF\\model\\au\\") ;

rem test1
rem inFile =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\three.js\\models\\gltf\\tests\\test1.fbx") ;
rem outDir/*3*/ =U("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\three.js\\models\\gltf\\tests\\") ;
rem test2
rem inFile =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\three.js\\models\\gltf\\tests\\test2.fbx") ;
rem outDir/*3*/ =U ("C:\\Users\\cyrille\\Documents\\Visual Studio 2013\\Projects\\glTF\\three.js\\models\\gltf\\tests\\") ;
