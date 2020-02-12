@echo off
mkdir build
pushd build
start "" cmd /c "cl /Zi /EHsc ..\main.cpp & pause"
popd
