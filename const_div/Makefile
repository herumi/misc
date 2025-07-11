MCL_USE_OMP=1
MCL_USE_GMP_LIB=0
include ../../mcl/common.mk
INC_DIR= -I ./ -I ../../xbyak -I ../../cybozulib/include
ifeq ($(ARCH),arm64)
INC_DIR+= -I ../../xbyak_aarch64/
LDFLAGS+= -L ../../xbyak_aarch64/lib -lxbyak_aarch64
endif

CFLAGS += $(INC_DIR)
HEADER=constdiv.hpp
VPATH=

SRC=div7_test.cpp
ALL_SRC=$(SRC) div7.cpp
EXE=$(SRC:.cpp=.exe)



TARGET=$(EXE)
all:$(TARGET)

.SUFFIXES: .cpp

%.o: %.cpp $(HEADER)
	$(CXX) -c -o $@ $< $(CFLAGS) -MMD -MP -MF $(@:.o=.d)

%.exe: %.o div7.o
	$(CXX) -o $@ $< $(LDFLAGS) div7.o

clean:
	$(RM) *.o *.d $(TARGET) *.exe

test: $(EXE)
	./$(EXE)

test_alld: $(EXE)
	./$(EXE) -alld

.PHONY: test clean

# don't remove these files automatically
.SECONDARY: $(ALL_SRC:.cpp=.o)
