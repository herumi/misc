CYBOZU_DIR?=../../cybozulib/include

CFLAGS+=-I $(CYBOZU_DIR)
test: gf256_test.exe
	./$<

gf256_test.exe: gf256_test.cpp
	$(CXX) -o $@ $< $(CFLAGS)

clean:
	$(RM) *.exe *.h
