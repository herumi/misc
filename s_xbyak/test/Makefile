PYTHON?=python3
DIFF_OPT=-urN

test:
	make test1 FILE=gen_ff_x64
	make test1 FILE=gen_bint_x64
	make test1 FILE=string
	make test1 FILE=misc
#	$(PYTHON) misc.py

test_misc:
	$(PYTHON) misc.py

test1:
	$(PYTHON) $(FILE).py -m gas > a.txt
	diff $(DIFF_OPT) $(FILE)_gas.txt a.txt
	$(PYTHON) $(FILE).py -win > a.txt
	diff $(DIFF_OPT) $(FILE)_win.txt a.txt
	$(PYTHON) $(FILE).py -win -m masm > a.txt
	diff $(DIFF_OPT) $(FILE)_masm.txt a.txt

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
	$(PYTHON) string.py -m nasm > string.asm
	nasm -f elf64 string.asm
endif
ifeq ($(MODE),gas)
	$(PYTHON) string.py -m gas > string.S
	$(CXX) -c string.S
endif
	$(CXX) -o $@ mie_string_test.o string.o $(CFLAGS)

mie_string_test: ./mie_string_test.exe
	echo $(MODE)
	./mie_string_test.exe

clean:
	rm -f a.txt *.o *.S *.exe *.d

.PHONY: clean test

# don't remove these files automatically
.SECONDARY: $(addprefix $(OBJ_DIR)/, $(ALL_SRC:.cpp=.o))
