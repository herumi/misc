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

def gen_test2():
	with FuncProc('test2'):
		with StackFrame(1) as sf:
			x = sf.p[0]
			L0 = Label()
			L1 = Label()
			L2 = Label()
			exitL = Label()
			mov(eax, 100)
			cmp(x, 0)
			je(L0)
			cmp(x, 1)
			je(L1)
			cmp(x, 2)
			je(L2)
			mov(eax, 123)
			jmp(exitL)
			L(L0)
			mov(eax, 0)
			jmp(exitL)
			L(L2)
			mov(eax, 202)
			jmp(exitL)
			L(L1)
			mov(eax, 101)
			jmp(exitL)
			xor_(eax, eax)
			L(exitL)
			ret()

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

gen_test2()

termOutput()
