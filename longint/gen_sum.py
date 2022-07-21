from gen_x86asm import *
import argparse

def gen_sum(name):
	with FuncProc(name):
		with StackFrame(1, 1) as sf:
			n = sf.p[0]
			i = sf.t[0]
			xor_(rax, rax)
			xor_(i, i)
			lpL = Label()
			L(lpL)
			add(rax, i)
			inc(i)
			cmp(i, n)
			jb(lpL)

def gen_test1():
	with FuncProc('test1'):
		with StackFrame(1) as sf:
			x = sf.p[0]
			exitL = Label()
			mov(eax, 0)
			cmp(x, 3)
			jb(exitL)
			mov(eax, 1)
			L(exitL)

parser = argparse.ArgumentParser()
parser.add_argument('-win', '--win', help='output win64 abi', action='store_true')
parser.add_argument('-m', '--mode', help='output asm syntax', default='nasm')
param = parser.parse_args()

setWin64ABI(param.win)
init(param.mode)

segment('text')

gen_test1()

gen_sum('sum')
gen_sum('sum2')


termOutput()
