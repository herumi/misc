@echo off

rem python3 gen.py -m masm > func_s.masm
rem ml64 /c /nologo func_s.masm

python3 gen.py -m nasm -win > func_s.nasm
nasm -f win64 func_s.nasm

cl /EHsc /nologo test.cpp func_s.obj /I ../../cybozulib/include
test.exe
