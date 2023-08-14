MCL_DIR?=../../mcl
SRC=invmod.cpp

OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)
EXE=$(SRC:.cpp=.exe)

all: $(EXE)

test: $(EXE)
	./$(EXE)

CFLAGS+=-I $(MCL_DIR)/include -Ofast -DNDEBUG -Wall -Wextra
LDFLAGS+=-lmcl -L $(MCL_DIR)/lib -lgmp -lgmpxx

%.exe: %.o
	$(PRE)$(CXX) $< -o $@ $(LDFLAGS)

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

clean:
	$(RM) $(OBJ) $(DEP) $(EXE)

-include $(DEP)
.SECONDARY: $(OBJ)
