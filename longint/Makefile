MCL_DIR?=../../mcl
include $(MCL_DIR)/common.mk
TARGET=code.exe

ALL_SRC=code.cpp
CFLAGS+=-I $(MCL_DIR)/include -I $(MCL_DIR)/src
LDFLAGS+=-lgmp -lgmpxx

all: $(TARGET)

%.o: %.cpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

%.exe: %.o
	$(PRE)$(CXX) $< -o $@ $(LDFLAGS)

DEPEND_FILE=$(addsuffix .d,$(basename $(ALL_SRC)))
-include $(DEPEND_FILE)

# don't remove these files automatically
.SECONDARY: $(ALL_SRC:.cpp=.o)
