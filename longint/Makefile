MCL_DIR?=../../mcl
include $(MCL_DIR)/common.mk
TARGET=mcl_test.exe

LIB_SRC=mcl.cpp
LIB_OBJ=$(LIB_SRC:.cpp=.o) $(MCL_DIR)/obj/bint64.o
LIB=libmcl.a
ALL_SRC=$(LIB_SRC) mcl_test.cpp
CFLAGS+=-I $(MCL_DIR)/include -I $(MCL_DIR)/src
LDFLAGS+=-lgmp -lgmpxx
BITINT_ASM=bitint-amd64.asm

all: $(TARGET)

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

$(MCL_DIR)/obj/bint64.o:
	make -C $(MCL_DIR) obj/bint64.o

$(TARGET): mcl_test.o $(LIB) addfp.o
	$(PRE)$(CXX) $< -o $@ $(LDFLAGS) $(LIB) addfp.o $(MCL_DIR)/lib/libmcl.a

$(LIB): $(LIB_OBJ)
	$(AR) $(ARFLAGS) $@ $(LIB_OBJ)

test: $(TARGET)
	./$(TARGET)

bitint:
	clang++-12 -S -O2 -Wall -Wextra -S -masm=intel -std=c++17 -mbmi2 -madx -funroll-loops -DNDEBUG bitint_x64.cpp -I $(MCL_DIR)

$(BITINT_ASM): gen_x86asm.py gen_bitint_x64.py
	python3 gen_bitint_x64.py > $(BITINT_ASM)

%.o: %.asm
ifeq ($(OS),mac)
	nasm -f macho64 $<
else
	nasm -f elf64 $<
endif

bitint_test.exe: bitint_test.o $(BITINT_ASM:.asm=.o)
	$(CXX) -o $@ bitint_test.o $(BITINT_ASM:.asm=.o) -lgmp -lgmpxx

test_bitint: bitint_test.exe
	./bitint_test.exe

gen_sum.asm: gen_sum.py
	python3 $< -m nasm > $@

gen_sum.o: gen_sum.asm
	nasm -f elf64 -o $@ $<

sum.exe: sum.cpp gen_sum.o
	$(CXX) -o $@ sum.cpp gen_sum.o

addfp.o: addfp.nasm
	nasm -f elf64 -o $@ $<

addfp.nasm: gen_add.py
	python3 $< -m nasm > $@

test_sum: sum.exe
	./sum.exe

clean:
	$(RM) *.o *.d *.exe *.a *.s *.lst $(BITINT_ASM) gen_sum.asm

DEPEND_FILE=$(ALL_SRC:.cpp=.d)
-include $(DEPEND_FILE)

# don't remove these files automatically
.SECONDARY: $(ALL_SRC:.cpp=.o)
