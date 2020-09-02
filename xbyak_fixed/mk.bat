@echo off
set INC=-I../../xbyak -I../../cybozulib/include
set OUT=t1-out
cl /EHsc t1.cpp %INC%
t1.exe %OUT%.asm
nasm -fwin64 %OUT%.asm
cl /EHsc t2.cpp %OUT%.obj
t2.exe
