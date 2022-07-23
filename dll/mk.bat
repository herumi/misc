python3 gen.py -win -m masm > asm.asm
ml64 /c asm.asm
cl /c /O2 sub.cpp
link /nologo /DLL /OUT:bin\dll.dll asm.obj sub.obj /implib:dll.lib
cl /O2 main.cpp dll.lib /Fe:bin\main.exe
bin\main.exe
