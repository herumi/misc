def mk_mul(N):
	print('template<>')
	print(f'void mulT<{N}>(uint32_t *z, const uint32_t *x, const uint32_t *y)')
	print(f'{{')
	print(f'	uint64_t v, L, H;')
	print(f'	v = uint64_t(x[0]) * y[0];')
	print(f'	z[0] = uint32_t(v);')
	print(f'	L = 0;')
	print(f'	H = v >> 32;')
	for d in range(1, N * 2):
		print(f'	L = L >> 32;')
		print(f'	L += H;')
		print(f'	H = 0;')
		for i in range(max(0, d+1-N), min(d+1, N)):
			print(f'	v = uint64_t(x[{i}]) * y[{d - i}];')
			print(f'	L += uint32_t(v);')
			print(f'	H += uint32_t(v >> 32);')
		print(f'	z[{d}] = uint32_t(L);')
	print(f'}}')

def mk_sqr(N):
	print('template<>')
	print(f'void sqrT<{N}>(uint32_t *y, const uint32_t *x)')
	print(f'{{')
	print(f'	uint64_t v, L, H, L2, H2;')
	print(f'	v = uint64_t(x[0]) * x[0];')
	print(f'	y[0] = uint32_t(v);')
	print(f'	L = 0;')
	print(f'	H = v >> 32;')
	for d in range(1, N * 2):
		print(f'	L = L >> 32;')
		print(f'	L += H;')
		print(f'	H = 0;')
		h = d // 2
		if (d % 2) == 0:
			print(f'	v = uint64_t(x[{h}]) * x[{h}];')
			print(f'	L += uint32_t(v);')
			print(f'	H += uint32_t(v >> 32);')
		print(f'	L2 = 0;')
		print(f'	H2 = 0;')
		for i in range(max(0, d+1-N), min(d+1, N)):
			if i >= d - i:
				break
			print(f'	v = uint64_t(x[{i}]) * x[{d - i}];')
			print(f'	L2 += uint32_t(v);')
			print(f'	H2 += uint32_t(v >> 32);')
		print(f'	L += L2 * 2;')
		print(f'	H += H2 * 2;')
		print(f'	y[{d}] = uint32_t(L);')
	print(f'}}')



def main():
	print("""
	template<size_t N>
	void mulT(uint32_t *z, const uint32_t *x, const uint32_t *y);""")
	print("""
	template<size_t N>
	void sqrT(uint32_t *y, const uint32_t *x);""")
	for i in [1, 2, 3, 8, 12]:
		mk_mul(i)
	for i in [1, 2, 3, 8, 12]:
		mk_sqr(i)

main()
