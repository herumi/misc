from gen_x86asm import *

def gen_add(N):
	if N == 0:
		xor_(eax, eax)
		ret()
		return


gen_add(0)
