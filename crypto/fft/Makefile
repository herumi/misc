MCL_DIR?=../../../mcl
ntt.exe: ntt.cpp
	g++ -Wall -Wextra -o ntt.exe ntt.cpp -lmcl -L$(MCL_DIR)/lib -I$(MCL_DIR)/include

test: ntt.exe
	./ntt.exe

