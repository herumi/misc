MCL_DIR?=../../mcl
include $(MCL_DIR)/common.mk
TARGET=mcl_test.exe

LIB_SRC=mcl.cpp
LIB_OBJ=$(LIB_SRC:.cpp=.o)
LIB=libmcl.a
ALL_SRC=$(LIB_SRC) mcl_test.cpp
CFLAGS+=-I $(MCL_DIR)/include -I $(MCL_DIR)/src
LDFLAGS+=-lgmp -lgmpxx

all: $(TARGET)

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

mcl_test.exe: mcl_test.o $(LIB)
	$(PRE)$(CXX) $< -o $@ $(LDFLAGS) $(LIB)

$(LIB): $(LIB_OBJ)
	$(AR) $@ $(LIB_OBJ)

test: $(TARGET)
	./$(TARGET)

bitint:
	clang++-12 -S -O2 -Wall -Wextra -S -masm=intel -std=c++17 -mbmi2 -madx -DNDEBUG bitint_x64.cpp -I $(MCL_DIR)

bitint.asm: gen_x86asm.py gen_bitint_x64.py
	python3 gen_bitint_x64.py > bitint.asm

bitint.o: bitint.asm
	nasm -f elf64 bitint.asm

bitint_test.exe: bitint_test.o bitint.o
	$(CXX) -o $@ bitint_test.o bitint.o -lgmp -lgmpxx

test_bitint: bitint_test.exe
	./bitint_test.exe

clean:
	$(RM) *.o *.d *.exe *.a *.s *.lst

DEPEND_FILE=$(ALL_SRC:.cpp=.d)
-include $(DEPEND_FILE)

# don't remove these files automatically
.SECONDARY: $(ALL_SRC:.cpp=.o)
