from gen_x86asm import *

def gen_add(N):
	align(16)
	name = f'mclb_add{N}:'
	print(name)
	print('_' + name)
	with StackFrame(3) as sf:
		if N == 0:
			xor_(eax, eax)
			return
		else:
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

#setWin64ABI(True)
gen_add(4)
