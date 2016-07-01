@echo off
nasm -fwin64 test.asm
cl t.cpp test.obj
sde -- t.exe
