MKL_DIR=/opt/intel/mkl/lib/intel64/
MKL_LIB=-L /opt/intel/mkl/lib/intel64/ -Wl,--start-group ${MKL_DIR}/libmkl_cdft_core.a ${MKL_DIR}/libmkl_intel_lp64.a ${MKL_DIR}/libmkl_sequential.a ${MKL_DIR}/libmkl_core.a ${MKL_DIR}/libmkl_blacs_intelmpi_lp64.a -Wl,--end-group -lpthread -ldl
CFLAGS=-I /opt/intel/mkl/include -I ../../TFHEpp/thirdparties/spqlios
LDFLAGS=../../TFHEpp/build/thirdparties/spqlios/libspqlios.a $(MKL_LIB)

test: test.cpp
	$(CXX) test.cpp -o $@ $(CFLAGS) $(LDFLAGS)
