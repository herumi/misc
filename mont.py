import sys
L=64
MASK = (1 << L) - 1

def getMontgomeryCoeff(p):
	pLow = p & MASK
	if False:
		ret = 0
		t = 0
		x = 1
		for i in range(L):
			if t % 2 == 0:
				t += pLow
				ret += x
			t >>= 1
			x <<= 1
	else:
		ret = 0
		t = 0
		for i in range(L):
			if (t & (2**i)) == 0:
				t += pLow << i
				ret += 1 << i
	return ret

class Montgomery:
	def __init__(self, p):
		self.p = p
		self.ip = getMontgomeryCoeff(p)
		self.pn = (len(bin(p)) - 2 + L - 1) // L
		self.M = 2**L
		self.iM = (self.p * self.ip + 1) // self.M
		self.Z = pow(self.M, self.pn, self.p)
		self.iZ = pow(self.iM, self.pn, self.p)

		assert self.M * self.iM - self.p * self.ip == 1
		assert (self.Z * self.iZ) % self.p == 1
#		R = 1
#		self.R = (1 << (self.pn * L)) % p
		self.Z2 = (self.Z * self.Z) % p
	def put(self):
		print(f'pn={self.pn}')
		print(f'p ={hex(self.p)}')
		print(f'ip={hex(self.ip)}')
		print(f'M ={hex(self.M)}')
		print(f'iM={hex(self.iM)}')
		print(f'Z ={hex(self.Z)}')
		print(f'iZ={hex(self.iZ)}')
		print(f'Z2={hex(self.Z2)}')
	def mod(self, x):
		y = x
		for i in range(self.pn):
			q = ((y & MASK) * self.ip) & MASK
			y += q * self.p
			y >>= L
		if y >= self.p:
			y -= self.p
		return y
	def mont(self, x, y):
		"""
		if t <= 2p-1, then
		t <= (2p-1) + (p-1)(M-1) + (M-1)p = 2p-1+pM-p-M+1+pM-p=2pM-M
		t/M <= 2p-1
		"""
		t = 0
		for i in range(self.pn):
			t += x * ((y >> (L * i)) & MASK)
			q = ((t & MASK) * self.ip) & MASK
			t += q * self.p
			t >>= L
		if t >= self.p:
			t -= self.p
		return t
	def toMont(self, x):
		return self.mont(x, self.Z2)
	def fromMont(self, x):
		return self.mont(x, 1)

	def mont_explicit(self, x, y):
		return (x * y * self.iZ) % self.p
	def toMont_explicit(self, x):
		return (x * self.Z) % self.p
	def fromMont_explicit(self, x):
		return (x * self.iZ) % self.p

pTbl = [
	# BN254 p, r
	0x2523648240000001ba344d8000000007ff9f800000000010a10000000000000d,
	0x2523648240000001ba344d80000000086121000000000013a700000000000013,
	# Fp256BN p, r
	0xfffffffffffcf0cd46e5f25eee71a49e0cdc65fb1299921af62d536cd10b500d,
	0xfffffffffffcf0cd46e5f25eee71a49f0cdc65fb12980a82d3292ddbaed33013,
	# BLS12-381 p, r
	0x1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab,
	0x73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001,
	# 511 bit
	0x65b48e8f740f89bffc8ab0d15e3e4c4ab42d083aedc88c425afbfcc69322c9cda7aac6c567f35507516730cc1f0b4f25c2721bf457aca8351b81b90533c6c87b,
]

for p in pTbl:
	mont = Montgomery(p)
	mont.put()

for x in range(1, 100, 11):
#	print(f'x={x} {mont.fromMont(mont.toMont(x))}')
	for y in range(x, x + 100, 11):
		xx =mont.toMont(x)
		xx2 = mont.toMont_explicit(x)
		if xx != xx2:
			print(f'ERR xx={xx} xx2={xx2}')
		yy = mont.toMont(y)
		zz = mont.mont(xx, yy)
		zz2 = mont.mont_explicit(xx, yy)
		if zz != zz2:
			print(f'ERR2 zz={zz} zz2={zz2}')
		z = mont.fromMont(zz)
		z2 = mont.fromMont_explicit(zz)
		if z != z2:
			print(f'ERR3 z={z} z2={z2}')
		xy = (x * y) % mont.p
		if xy != z:
			print(f'ERR4 x={x} y={y} xy={xy}, z={z}')
			sys.exit(1)
	print('ok')
