MCL_DIR?=../../mcl
include $(MCL_DIR)/common.mk
TARGET=main.exe

LIB_SRC=code.cpp
LIB_OBJ=$(LIB_SRC:.cpp=.o)
CODE_LIB=libcode.a
ALL_SRC=$(LIB_SRC) main.cpp
CFLAGS+=-I $(MCL_DIR)/include -I $(MCL_DIR)/src
LDFLAGS+=-lgmp -lgmpxx

all: $(TARGET)

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

main.exe: main.o $(CODE_LIB)
	$(PRE)$(CXX) $< -o $@ $(LDFLAGS) $(CODE_LIB)

$(CODE_LIB): $(LIB_OBJ)
	$(AR) $@ $(LIB_OBJ)

test: $(TARGET)
	./$(TARGET)

clean:
	$(RM) *.o *.d *.exe *.a

DEPEND_FILE=$(ALL_SRC:.cpp=.d)
-include $(DEPEND_FILE)

# don't remove these files automatically
.SECONDARY: $(ALL_SRC:.cpp=.o)
