@echo off
python3 gen.py -m masm > func_s.masm
ml64 /c /nologo func_s.masm
rem python3 gen.py -m nasm -win > func_s.nasm
rem nasm -f win64 func_s.nasm
cl /EHsc /nologo test.cpp func_s.obj
test.exe
