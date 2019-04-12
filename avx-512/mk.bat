@echo off
set SRC=%1
set EXE=%SRC:~0,-4%.exe
cl /EHsc /arch:AVX2 -I ../../xbyak -I ../../cybozulib/include /W4 %SRC%
..\..\intel\sde -future -- %EXE%
