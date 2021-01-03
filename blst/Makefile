BLST_DIR=../../blst
all: bench

bench: bench.cpp
	$(CXX) -Ofast -DNDEBUG -o bench bench.cpp -I $(BLST_DIR)/bindings/ -I ../../cybozulib/include -lblst -L $(BLST_DIR)

test: bench
	./bench
