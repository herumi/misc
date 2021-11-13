SRC=add.cpp
OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)
EXE=$(SRC:.cpp=.exe)
all: $(EXE)

XBYAK_DIR?=../../fujitsu/xbyak_aarch64
LDFLAGS=-L $(XBYAK_DIR)/lib -lxbyak_aarch64
CFLAGS=-std=c++11 -g -DNDEBUG -Wall -Wextra -I $(XBYAK_DIR)
#CFLAGS+=-march=armv8.2-a+sve
ifneq ($(DEBUG),1)
CFLAGS+=-O2
endif

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) -MMD -MP -MF $(@:.o=.d)

-include $(DEP)

%.exe: %.o
	$(CXX) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(EXE) $(OBJ) $(DEP)

test: ./add.exe
	./add.exe

.PHONY: clean

.SECONDARY: $(OBJ)
