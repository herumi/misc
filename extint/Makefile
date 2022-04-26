TARGET=add256.o mul256.o

ALL: $(TARGET)
.SUFFIXES: .cpp .d .exe .c .o .ll .bc

LLVM_VER?=-12
CXX=clang++$(LLVM_VER)
LLVM_DIS?=llvm-dis$(LLVM_VER)

CFLAGS=-Wall -Wextra -O2 -DNDEUBG -I ../../mcl/include -fpic -std=c++17

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.ll
	$(CXX) -c -o $@ $< $(CFLAGS)

%.exe: %.o
	$(CXX) -o $@ $< $(CFLAGS) -lgmp -lgmpxx

bitint_if.bc: bitint_if.cpp bitint.hpp bitint_if.hpp
	$(CXX) -c -emit-llvm -o $@ $< $(CFLAGS)

bitint_llvm_test.exe: bitint_llvm_test.o bitint_if.ll
	$(CXX) -o $@ $< $(CFLAGS) -lgmp -lgmpxx bitint_if.ll

%.ll: %.bc
	$(LLVM_DIS) -o $@ $<

bitint_test: bitint_test.exe
	./$<

bitint_llvm_test: bitint_llvm_test.exe
	./$<

clean:
	$(RM) -rf *.o *.bc *.exe *.bc bitint.ll
