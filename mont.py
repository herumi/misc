import sys
L=64
MASK = (1 << L) - 1

def getMontgomeryCoeff(p):
	pLow = p & MASK
	ret = 0
	t = 0
	x = 1
	for i in range(L):
		if t % 2 == 0:
			t += pLow
			ret += x
		t >>= 1
		x <<= 1
	return ret

class Montgomery:
	def __init__(self, p):
		self.p = p
		self.rp = getMontgomeryCoeff(p)
		self.pn = (len(bin(p)) - 2 + L - 1) // L
		R = 1
		self.R = (1 << (self.pn * L)) % p
		self.RR = (self.R * self.R) % p
	def put(self):
		print(f'p={hex(self.p)}')
		print(f'rp={hex(self.rp)}')
		print(f'pn={self.pn}')
		print(f'R={hex(self.R)}')
#		print(f'RR={hex(self.RR)}')
	def mod(self, x):
		y = x
		for i in range(self.pn):
			q = ((y & MASK) * self.rp) & MASK
			y += q * self.p
			y >>= L
		if y >= self.p:
			y -= self.p
		return y
	def mul(self, x, y):
		return self.mod(x * y)
	def toMont(self, x):
		return self.mul(x, self.RR)
	def fromMont(self, x):
		return self.mul(x, 1)

pTbl = [
	# BN254 p, r
	0x2523648240000001ba344d8000000007ff9f800000000010a10000000000000d,
	0x2523648240000001ba344d80000000086121000000000013a700000000000013,
	# Fp256BN p, r
	0xfffffffffffcf0cd46e5f25eee71a49e0cdc65fb1299921af62d536cd10b500d,
	0xfffffffffffcf0cd46e5f25eee71a49f0cdc65fb12980a82d3292ddbaed33013,
]

for p in pTbl:
	mont = Montgomery(p)
	mont.put()

for x in range(1, 100, 11):
#	print(f'x={x} {mont.fromMont(mont.toMont(x))}')
	for y in range(x, x + 100, 11):
		xx =mont.toMont(x)
		yy = mont.toMont(y)
		zz = mont.mul(xx, yy)
		z = mont.fromMont(zz)
		xy = (x * y) % mont.p
		if xy != z:
			print(f'ERR x={x} y={y} xy={xy}, z={z}')
			sys.exit(1)
	print('ok')
