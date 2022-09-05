L=64

def getMontgomeryCoeff(p):
	pLow = p & ((1 << L) - 1)
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
