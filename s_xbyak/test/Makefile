test:
	make test1 FILE=gen_ff_x64
	make test1 FILE=gen_bint_x64
	make test1 FILE=string

test1:
	python3 $(FILE).py -m gas > a.txt
	diff -urN $(FILE)_gas.txt a.txt
	python3 $(FILE).py -win > b.txt
	diff -urN $(FILE)_win.txt b.txt

test_string_run:
	make clean
	make mie_string_test
	make clean
	make mie_string_test MODE=gas

SRC=mie_string_test.cpp

MIE_STRING_DIR?=../../../mie_string/
CYBOZULIB_DIR?=../../../cybozulib/
CFLAGS=-O2 -DNDEBUG -mavx2 -I $(MIE_STRING_DIR) -I $(CYBOZULIB_DIR)/include

VPATH=$(MIE_STRING_DIR)

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)
DEPEND_FILE=$(addsuffix .d,$(SRC))
-include $(DEPEND_FILE)

MODE?=nasm
mie_string_test.exe: string.py mie_string_test.o
ifeq ($(MODE),nasm)
	python3 string.py -m nasm > string.asm
	nasm -f elf64 string.asm
endif
ifeq ($(MODE),gas)
	python3 string.py -m gas > string.S
	$(CXX) -c string.S
endif
	$(CXX) -o $@ mie_string_test.o string.o $(CFLAGS)

mie_string_test: ./mie_string_test.exe
	echo $(MODE)
	./mie_string_test.exe

clean:
	rm -f a.txt b.txt *.o *.S *.exe *.d

.PHONY: clean test

# don't remove these files automatically
.SECONDARY: $(addprefix $(OBJ_DIR)/, $(ALL_SRC:.cpp=.o))
