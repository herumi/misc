BLST_DIR=../../blst
all: bench

bench: bench.cpp
	$(CXX) -Ofast -DNDEBUG -o bench bench.cpp -I $(BLST_DIR)/bindings/ -I ../../cybozulib/include -lblst -L $(BLST_DIR)

test: bench
	./bench


mcl: mcl.cpp
	$(CXX) -Ofast -DNDEBUG -o mcl mcl.cpp -I ../../mcl/include ../../mcl/lib/libmcl.a
