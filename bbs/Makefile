MCL_DIR?=../../mcl
include $(MCL_DIR)/common.mk
SRC=bbs_test.cpp
OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)
TARGET=$(SRC:.cpp=.exe)

all: $(TARGET)

CFLAGS+=-I $(MCL_DIR)/include -I ./
LDFLAGS+=-L $(MCL_DIR)/lib -L ./

libbbs.a: bbs.o
	$(AR) $(ARFLAGS) $@ $<

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

%.exe: %.o libbbs.a
	$(PRE)$(CXX) $< -o $@ -lmcl -lbbs $(LDFLAGS)

test: $(TARGET)
	./$(TARGET)

-include $(DEP)

clean:
	$(RM) $(TARGET) $(OBJ) $(DEP) *.a

.PHONY: clean

# don't remove these files automatically
.SECONDARY: $(OBJ)
