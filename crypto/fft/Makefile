MCL_DIR?=../../../mcl
%.exe: %.cpp
	g++ -Wall -Wextra -o $@ $< -lmcl -L$(MCL_DIR)/lib -I$(MCL_DIR)/include

test: ntt_test.exe
	./$<

clean:
	rm -rf *.exe
