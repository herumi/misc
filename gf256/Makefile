ARCH?=$(shell uname -m)
CYBOZU_DIR?=../../cybozulib/include
XBYAK_DIR?=../../xbyak/

CFLAGS+=-DNDEBUG -O2 -Wall -Wextra -I $(CYBOZU_DIR) -I $(XBYAK_DIR)
test_emu: gf256_test.exe
	sde -spr -- ./$<

ifeq ($(ARCH),x86_64)
ASM_OBJ+=gf256_gfni.o
endif

test: gf256_test.exe
	./$<

update: gf256_gfni.S gf256_gfni.asm

gf256_test.exe: gf256_test.o $(ASM_OBJ)
	$(CXX) -o $@ $^ $(CFLAGS)

%.o: %.cpp gf256.hpp mul.h
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.S
	$(CXX) -c -o $@ $< $(CFLAGS)

gf256_gfni.S: gen_asm.py
	python3 gen_asm.py -m gas > $@

gf256_gfni.asm: gen_asm.py
	python3 gen_asm.py -m masm > $@

clean:
	$(RM) *.exe *.o *.S
