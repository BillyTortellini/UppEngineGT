@echo off
mkdir build
pushd build
start "" cmd /c "cl /std:c++latest /Zi /EHsc ..\main.cpp & pause"
popd
