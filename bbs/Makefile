MCL_DIR?=../../mcl
SRC=bbs_test.cpp
OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)
TARGET=$(SRC:.cpp=.exe)

all: $(TARGET)

MCL_LIB=-lmcl
CFLAGS=-O2 -Wall -Wextra -I $(MCL_DIR)/include
LDFLAGS=-L $(MCL_DIR)/lib

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

%.exe: %.o
	$(PRE)$(CXX) $< -o $@ $(MCL_LIB) $(LDFLAGS)

test: $(TARGET)
	./$(TARGET)

-include $(DEP)

clean:
	$(RM) $(TARGET) $(OBJ) $(DEP)

.PHONY: clean

# don't remove these files automatically
.SECONDARY: $(OBJ)
