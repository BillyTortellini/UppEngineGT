@echo off
mkdir ..\build
pushd ..\build
start "" cmd /c "cl /Zi /EHsc ..\code\main.cpp & pause"
popd