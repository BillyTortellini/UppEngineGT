@echo off

pushd ..\build
cl /EHsc /Zi /FC ..\code\main.cpp && (start "" cmd /c "cd .. & cls & build\main.exe & pause") || pause
popd
