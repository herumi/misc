@echo off
rm -f bin\*.dll bin\*.exe *.obj *.lib
rem python3 gen.py -win -m masm > asm.asm
rem ml64 /c asm.asm
python3 gen.py -win -m nasm > asm.asm
nasm -fwin64 asm.asm
cl /c /O2 sub.cpp
link /nologo /DLL /OUT:bin\suba.dll asm.obj sub.obj /implib:suba.lib
dumpbin /exports suba.lib
cl /O2 main.cpp suba.lib /Fe:bin\main.exe
bin\main.exe

lib /nologo /OUT:subb.lib /nodefaultlib sub.obj asm.obj
cl /O2 main.cpp subb.lib /Fe:bin\mainb.exe
bin\mainb.exe
