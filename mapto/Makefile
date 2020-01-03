a.out: test.cpp
	g++  test.cpp -I ../../mcl/include/ -lmcl -L../../mcl/lib -lcrypto -lgmp -lgmpxx

test: a.out
#	-python3 ../../bls12-381_hash/python-impl/opt_swu_g2.py
	./a.out
