@echo off

mkdir ..\build
pushd ..\build
start "" cmd /c "cl -nologo -Od -W4 -WX -wd4100 -wd4189 -wd4996 -wd4459 -GR- -EHa- -Oi -Z7 -MT -FC -D_USRDLL -D_WINDLL ..\code\win32\platformBuild.cpp -link -DLL -OUT:game.dll || pause & (copy game.dll game_tmp.dll)"
popd
