include ../../mcl/common.mk
TARGET=add256.o mul256.o

ALL: $(TARGET)
.SUFFIXES: .cpp .d .exe .c .o .ll .bc

LLVM_VER?=-12
CXX=clang++$(LLVM_VER)
LLVM_DIS?=llvm-dis$(LLVM_VER)
LLVM_LLC=llc$(LLVM_VER)
LLVM_OPT=opt$(LLVM_VER)
BIT?=64

CFLAGS=-Wall -Wextra -O2 -DNDEBUG -I ../../mcl/include -fpic -std=c++17

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.ll
	$(CXX) -c -o $@ $< $(CFLAGS)

%.exe: %.o
	$(CXX) -o $@ $< $(CFLAGS) -lgmp -lgmpxx

bitint_if32.bc: bitint_if.cpp bitint.hpp bitint_if.hpp
	$(CXX) -c -emit-llvm -o $@ $< $(CFLAGS) -DMCL_VINT_UNIT=4

bitint_if64.bc: bitint_if.cpp bitint.hpp bitint_if.hpp
	$(CXX) -c -emit-llvm -o $@ $< $(CFLAGS) -DMCL_VINT_UNIT=8

bitint_llvm_test.exe: bitint_llvm_test.o bitint_if$(BIT).ll
	$(CXX) -o $@ $< $(CFLAGS) -lgmp -lgmpxx bitint_if$(BIT).ll

$(ASM_SRC): $(ASM_SRC_DEP)
	$(LLVM_OPT) -O3 -o - $< -march=$(CPU) | $(LLVM_LLC) -O3 -o $@ $(LLVM_FLAGS)

bitint_if32.ll: bitint_if32.bc
	$(LLVM_DIS) -o $@ $<

bitint_if64.ll: bitint_if64.bc
	$(LLVM_DIS) -o $@ $<

bitint_test: bitint_test.exe
	./$<

bitint_llvm_test: bitint_llvm_test.exe
	./$<

clean:
	$(RM) -rf *.o *.bc *.exe *.bc *.ll
