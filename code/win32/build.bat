@echo off

mkdir ..\..\build
pushd ..\..\build
start "" cmd /c "cl /Zi /FC  ..\code\win32\main.cpp User32.lib Gdi32.lib & pause"
popd
