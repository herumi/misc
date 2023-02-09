MCL_DIR?=../../../mcl
ntt_naieve.exe: ntt_naieve.cpp
	g++ -Wall -Wextra -o $@ $< -lmcl -L$(MCL_DIR)/lib -I$(MCL_DIR)/include

test: ntt_naieve.exe
	./ntt_naieve.exe

