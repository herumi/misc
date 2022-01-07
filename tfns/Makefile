MCL_DIR?=../../mcl/
include $(MCL_DIR)common.mk

ALL_SRC=tfns_test.cpp

TARGET=tfns
all: $(TARGET)

CFLAGS+=-I $(MCL_DIR)include -I ./
ifeq ($(OS),mac)
  CFLAGS+=-Xpreprocessor -fopenmp
  LDFLAGS+=-lomp
else
  CFLAGS+=-fopenmp
  LDFLAGS+=-fopenmp
endif

MCL_LIB=$(MCL_DIR)lib/libmcl.a

$(MCL_LIB):
	$(MAKE) -C $(MCL_DIR) lib/libmcl.a

%.o: %.cpp tfns.hpp
	$(PRE)$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

tfns_test: tfns_test.o $(MCL_LIB)
	$(PRE)$(CXX) $< -o $@ $(MCL_LIB) $(LDFLAGS) -lpthread

test: tfns_test
	./tfns_test

clean:
	rm -rf $(TARGET) *.o tfns_test *.d

DEPEND_FILE=$(ALL_SRC:.cpp=.d)
-include $(DEPEND_FILE)

# don't remove these files automatically
.SECONDARY: $(ALL_SRC:.cpp=.o)

