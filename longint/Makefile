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

clean:
	$(RM) *.o *.d *.exe *.a

DEPEND_FILE=$(ALL_SRC:.cpp=.d)
-include $(DEPEND_FILE)

# don't remove these files automatically
.SECONDARY: $(ALL_SRC:.cpp=.o)
