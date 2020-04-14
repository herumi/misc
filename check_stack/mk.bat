@echo off
@echo with __chkstk
nasm -fwin64 -DUSE_CHKSTK f.asm
cl /EHsc /Ox t.cpp f.obj
echo run
t.exe

@echo without __chkstk
nasm -fwin64 f.asm
cl /EHsc /Ox t.cpp f.obj
echo run
t.exe
