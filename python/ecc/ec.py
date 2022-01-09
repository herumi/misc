from fp import Fp
from fr import Fr

class Ec:
	a = Fp()
	b = Fp()
	r = 0

	# E : y^2 = x^3 + ax + b mod p. r is the order of E
	@classmethod
	def init(cls, a, b, r):
		print(f"a={a} b={b}")
		if type(a) is int:
			a = Fp(a)
		if type(b) is int:
			b = Fp(b)
		cls.a = a
		cls.b = b
		cls.r = r

	def __init__(self, x=None, y=None, doVerify=True):
		self.isZero = True
		if x is None and y is None:
			return
		if type(x) is int:
			x = Fp(x)
		if type(y) is int:
			y = Fp(y)
		self.x = x
		self.y = y
		self.isZero = False
		if doVerify and not self.isValid():
			raise Exception(f"isValid x={x}, y={y}")
	def __str__(self):
		if self.isZero:
			return "O"
		else:
			return f"({self.x}, {self.y})"
	def __eq__(self, rhs):
		if self.isZero:
			return rhs.isZero
		if rhs.isZero:
			return False
		return self.x == rhs.x and self.y == rhs.y

	def isValid(self):
		if self.isZero:
			return True
		return self.y * self.y == (self.x * self.x + self.a) * self.x + self.b

	def __neg__(self):
		if self.isZero:
			return self
		return Ec(self.x, -self.y, False)

	def __add__(self, rhs):
		if self.isZero:
			return rhs
		if rhs.isZero:
			return self
		if self.x == rhs.x:
			# P + (-P) = 0
			if self.y == -rhs.y:
				return Ec()
			# dbl
			L = self.x * self.x
			L = (L + L + L + self.a) / (self.y + self.y)
		else:
			L = (self.y - rhs.y) / (self.x - rhs.x)
		x3 = L * L - (self.x + rhs.x)
		y3 = L * (self.x - x3) - self.y
		return Ec(x3, y3, False)

	def __mul__(self, rhs):
		if type(rhs) is Fr:
			rhs = rhs.v
		elif type(rhs) is not int:
			raise Exception("bad type", rhs)
		if rhs == 0:
			return Ec()
		bs = bin(rhs)[2:]
		ret = Ec()
		for b in bs:
			ret += ret
			if b == '1':
				ret += self
		return ret

def initSecp256k1():
	p = 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f
	a = 0
	b = 7
	r = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141
	Fp.init(p)
	Fr.init(r)
	Ec.init(a, b, r)
	x = 0x79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798
	y = 0x483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8
	P = Ec(x, y)
	return P

def main():
	P = initSecp256k1()
	Q = Ec()
	for i in range(200):
		R = P * i
		assert Q == R, f"mul i={i}"
		Q += P
	assert (P * Ec.r).isZero, "order"
	a = 12345678932
	b = 98763445345
	aP = P * a
	bP = P * b
	assert aP * b == bP * a, "DH"

if __name__ == '__main__':
	main()
