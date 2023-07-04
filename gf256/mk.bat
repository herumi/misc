python3 gen_asm.py -m masm > gf256_gfni.asm
ml64 /c /nologo gf256_gfni.asm
cl /nologo /EHsc /Ox /DNDEBUG gf256_test.cpp gf256_gfni.obj -I ../../cybozulib/include -I ../../xbyak
gf256_test.exe
