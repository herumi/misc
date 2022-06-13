from gen_x86asm import *

def gen_add(N):
	proc(f'mclb_add{N}')
	if N == 0:
		xor_(eax, eax)
		ret()
		return
	with StackFrame(3) as sf:
		z = sf.p[0]
		x = sf.p[1]
		y = sf.p[2]
		for i in range(N):
			mov(rax, ptr(x + 8 * i))
			if i == 0:
				add(rax, ptr(y + 8 * i))
			else:
				adc(rax, ptr(y + 8 * i))
			mov(ptr(z + 8 * i), rax)
		setc(al)
		movzx(eax, al)

def gen_sub(N):
	proc(f'mclb_sub{N}')
	if N == 0:
		xor_(eax, eax)
		ret()
		return
	with StackFrame(3) as sf:
		z = sf.p[0]
		x = sf.p[1]
		y = sf.p[2]
		for i in range(N):
			mov(rax, ptr(x + 8 * i))
			if i == 0:
				sub(rax, ptr(y + 8 * i))
			else:
				sbb(rax, ptr(y + 8 * i))
			mov(ptr(z + 8 * i), rax)
		setc(al)
		movzx(eax, al)

def gen_mulUnit(N):
	proc(f'mclb_mulUnit{N}')
	if N == 0:
		xor_(eax, eax)
		ret()
		return
	with StackFrame(3, useRDX=True) as sf:
		z = sf.p[0]
		x = sf.p[1]
		y = sf.p[2]
		if N == 1:
			mov(rax, ptr(x))
			mul(y)
			mov(ptr(z), rax)
			mov(rax, rdx)


#setWin64ABI(True)
gen_mulUnit(1)

with StackFrame(4, 4, useRDX=True,useRCX=True) as sf:
	for e in sf.p:
		print(e)
	print('---')
	for e in sf.t:
		print(e)
