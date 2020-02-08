@echo off

mkdir build
pushd build
start "" cmd /c "cl /EHsc /Zi /FC  ..\test.cpp || pause"
popd
