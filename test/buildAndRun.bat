@echo off

mkdir build
pushd build
cl /EHsc /Zi /FC ..\test.cpp && (cls && test.exe) || pause
popd
