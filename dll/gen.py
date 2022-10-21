from gen_x86asm import *
import argparse

def gen_test():
	with FuncProc('fff'):
		with StackFrame(2) as sf:
			x = sf.p[0]
			y = sf.p[1]
			L1 = Label()
			L2 = Label()
			lea(eax, ptr(x + y))
			jmp(L2)
			L(L1)
			mov(rax, x)
			add(rax, y)
			ret()
			L(L2)
			jmp(L1)

parser = argparse.ArgumentParser()
parser.add_argument('-win', '--win', help='output win64 abi', action='store_true')
parser.add_argument('-m', '--mode', help='output asm syntax', default='nasm')
param = parser.parse_args()

setWin64ABI(param.win)
init(param.mode)

segment('text')

gen_test()

termOutput()
