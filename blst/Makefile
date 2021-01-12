BLST_DIR=../../blst
MCL_DIR=../../mcl
MCL_LIB=$(MCL_DIR)/lib/libmcl.a
all: bench $(MCL_LIB)

CFLAGS=-Ofast -DNDEBUG
CFLAGS+=-I $(BLST_DIR)/bindings
CFLAGS+=-I $(MCL_DIR)/include
LDFLAGS+=-lblst -L $(BLST_DIR)
LDFLAGS+=$(MCL_LIB)

$(MCL_LIB):
	make -C .$(MCL_DIR) MCL_STATIC_CODE=1 MCL_USE_GMP=0 lib/libmcl.a

bench: bench.cpp
	$(CXX) -o bench bench.cpp $(CFLAGS) $(LDFLAGS)

test: bench
	./bench

