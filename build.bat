@echo off

set warn=-W4 -wd4201 -wd4115 -wd4100 -wd4189 -wd4101
set libs=user32.lib d3d11.lib d3dcompiler.lib dxguid.lib gdi32.lib advapi32.lib dsound.lib

cls

pushd bin
cl -FC -Z7 -GS -RTCs -nologo %warn% ../main.c %libs%
popd