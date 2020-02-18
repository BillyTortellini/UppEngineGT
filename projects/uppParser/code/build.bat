@echo off

mkdir ..\build
pushd ..\build
start "" cmd /c "cl /Zi /FC /EHsc  ..\code\main.cpp & pause"
popd
