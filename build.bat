@echo off

set warn=-W4 -wd4201 -wd4115 -wd4100 -wd4189 -wd4101
set libs=user32.lib d3d11.lib d3dcompiler.lib dxguid.lib gdi32.lib

cls

pushd bin
cl -FC -Z7 -nologo %warn% ../main.c %libs%
popd