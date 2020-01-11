@echo off

mkdir ..\build
pushd ..\build
start "" cmd /c "cl /Zi /FC  ..\code\program.cpp User32.lib Gdi32.lib & pause"
popd
