CXX?=clang++
INC_DIR?=../../fujitsu/xbyak_aarch64/
LIB_DIR?=$(INC_DIR)/lib
CPPFLAGS=$(OPT) -Wall -I $(INC_DIR) -I ./include
ifneq ($(DEBUG),1)
CPPFLAGS+=-O2 -DNDEBUG
endif
LDFLAGS=-s -L $(LIB_DIR) -lxbyak_aarch64
SRC=double-vec.cpp int-vec.cpp float-vec.cpp float-test.cpp exp_v.cpp
OBJ=$(SRC:.cpp=.o)
EXE=$(SRC:.cpp=.exe)
DEP=$(SRC:.cpp=.d)
all: $(EXE)

-include $(DEP)

%.o: %.cpp
	$(CXX) -c $< $(CPPFLAGS) -MMD -MP -MF $(@:.o=.d)

%.exe: %.o
	$(CXX) -o $@ $< $(LDFLAGS)

emu:
	aarch64-linux-gnu-g++ float-test.cpp -I ../../fujitsu/xbyak_aarch64/
	env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 -cpu max,sve512=on ./a.out

clean:
	rm -rf $(EXE) *.o *.d

.SECONDARY: $(OBJ) $(DEP)
