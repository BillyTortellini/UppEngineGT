@echo off

IF NOT EXIST ..\..build mkdir ..\..\build
pushd ..\..\build
REM GR- runtime type info, EHa remove exceptions, Oi optimize intrinsics, Zi (Or /Z7) add debug info, -WX warnings=errors, -W4 or -Wall, FC full path, -Od disable optimization
start "" cmd /c "cl -IO:\libs\openGLExtensions -Od -nologo -W4 -WX -wd4101 -wd4189 -wd4100 -wd4996 -wd4459 -GR- -EHa- -Oi -Z7 -MT -FC  ..\code\win32\main.cpp User32.lib Gdi32.lib Winmm.lib Opengl32.lib || pause"
popd
