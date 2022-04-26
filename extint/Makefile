TARGET=add256.o mul256.o

ALL: $(TARGET)
.SUFFIXES: .cpp .d .exe .c .o .ll .bc

LLVM_VER?=-12
CXX=clang++$(LLVM_VER)
LLVM_DIS=llvm-dis$(LLVM_VER)

CFLAGS=-Wall -Wextra -O2 -DNDEUBG -I ../../mcl/include -fpic -std=c++17

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.ll
	$(CXX) -c -o $@ $< $(CFLAGS)

bitint.bc: bitint.cpp bitint.hpp
	$(CXX) -c -emit-llvm -o $@ $< $(CFLAGS)

%.ll: %.bc
	$(LLVM_DIS) -o $@ $<

bitint_test.exe: bitint_test.o bitint.ll bitint.o
	$(CXX) -o $@ $< $(CFLAGS) -lgmp -lgmpxx bitint.o

bitint_test.o: bitint_test.cpp bitint.hpp
	$(CXX) -c -o $@ $< $(CFLAGS)

bitint_test: bitint_test.exe
	./bitint_test.exe



clean:
	$(RM) -rf *.o *.bc *.exe *.bc bitint.ll
