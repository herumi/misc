BLST_DIR=../../blst
MCL_DIR=../../mcl
all: bench

CFLAGS=-Ofast -DNDEBUG
CFLAGS+=-I $(BLST_DIR)/bindings
CFLAGS+=-I $(MCL_DIR)/include
LDFLAGS+=-lblst -L $(BLST_DIR)
LDFLAGS+=$(MCL_DIR)/lib/libmcl.a

bench: bench.cpp
	$(CXX) -o bench bench.cpp $(CFLAGS) $(LDFLAGS)

test: bench
	./bench

