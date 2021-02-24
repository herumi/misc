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
		for i in range(min(d+1, N)):
			if d - i >= N:
				continue
			print(f'	v = uint64_t(x[{i}]) * y[{d - i}];')
			print(f'	L += uint32_t(v);')
			print(f'	H += uint32_t(v >> 32);')
		print(f'	z[{d}] = uint32_t(L);')
	print(f'}}')


print("""
template<size_t N>
void mulT(uint32_t *z, const uint32_t *x, const uint32_t *y);""")

for i in [1, 2, 3]:
	mk_mul(i)
