MCL_DIR?=../../mcl
CLANG?=clang++
CXX=$(CLANG)
CFLAGS=-g -Wall -Wextra -DNDEBUG -O2 -mbmi2
CFLAGS+=-I $(MCL_DIR)/include
LDFLAGS+=-L $(MCL_DIR)/lib -lmcl

SRC=inv_test.cpp
LL=inv.ll inv-org.ll
LL_OBJ=$(LL:.ll=.o)
OBJ=$(SRC:.cpp=.o) $(LL_OBJ)

test: inv_test.exe
	./$<

%.exe: %.o $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.ll
	$(CXX) -c -o $@ $< $(CFLAGS)

%.o: %.S
	$(CXX) -c -o $@ $<

#%.s: %.ll
#	$(CXX) -O2 -S -o $@ $<

inv.ll: gen_inv.py
	python3 gen_inv.py > inv.ll

clean:
	rm -rf *.o *.exe *.S inv.ll

.PHONY: test clean

.SECONDARY: $(OBJ)
