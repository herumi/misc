CYBOZU_DIR?=../../cybozulib/include

CFLAGS+=-I $(CYBOZU_DIR)
test: gf256_test.exe
	sde -grr -- ./$<

update: gf256_gfni.S gf256_gfni.asm

gf256_test.exe: gf256_test.cpp gf256_gfni.S
	$(CXX) -o $@ $^ $(CFLAGS)

gf256_gfni.S: gen_asm.py
	python3 gen_asm.py -m gas > $@

gf256_gfni.asm: gen_asm.py
	python3 gen_asm.py -m masm > $@

clean:
	$(RM) *.exe *.h
