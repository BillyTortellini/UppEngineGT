@echo off
pushd build
cl /EHsc /Zi /FC ..\main.cpp && (start "" cmd /c "cd .. & cls & build\main.exe &pause") || pause
popd
