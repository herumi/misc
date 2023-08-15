MCL_DIR?=../../mcl
include $(MCL_DIR)/common.mk
SRC=invmod.cpp invmod_gmp.cpp

OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)
EXE=$(SRC:.cpp=.exe)

all: $(EXE)

test: invmod.exe
	./$<

test_gmp: invmod_gmp.exe
	./$<

CFLAGS=-I $(MCL_DIR)/include -Wall -Wextra -g
LDFLAGS=-lmcl -L $(MCL_DIR)/lib -lgmp -lgmpxx
ifeq ($(DEBUG),1)
  CFLAGS+=-fsanitize=address
  LDFLAGS+=-fsanitize=address
else
  CFLAGS+=-O2 -DNDEBUG
endif

%.exe: %.o
	$(PRE)$(CXX) $< -o $@ $(LDFLAGS)

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

clean:
	$(RM) $(OBJ) $(DEP) $(EXE)

-include $(DEP)
.SECONDARY: $(OBJ)
