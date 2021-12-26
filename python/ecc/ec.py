from fp import Fp
from fr import Fr

class Ec:
	a_ = Fp()
	b_ = Fp()
	r_ = 0

	# E : y^2 = x^3 + ax + b mod p. r is the order of E
	@classmethod
	def init(cls, a, b, r):
		print(f"a={a} b={b}")
		if type(a) is int:
			a = Fp(a)
		if type(b) is int:
			b = Fp(b)
		cls.a_ = a
		cls.b_ = b
		cls.r_ = r

	def __init__(self, x=None, y=None, doVerify=True):
		self.isZero_ = True
		if x is None and y is None:
			return
		if type(x) is int:
			x = Fp(x)
		if type(y) is int:
			y = Fp(y)
		self.x_ = x
		self.y_ = y
		self.isZero_ = False
		if doVerify and not self.isValid():
			raise Exception(f"isValid x={x}, y={y}")
	def __str__(self):
		if self.isZero_:
			return "O"
		else:
			return f"({self.x_}, {self.y_})"
	def __eq__(self, rhs):
		if self.isZero_:
			return rhs.isZero_
		if rhs.isZero_:
			return False
		return self.x_ == rhs.x_ and self.y_ == rhs.y_

	def isValid(self):
		if self.isZero_:
			return True
		return self.y_ * self.y_ == (self.x_ * self.x_ + self.a_) * self.x_ + self.b_

	def isZero(self):
		return self.isZero_

	def __neg__(self):
		if self.isZero():
			return self
		return Ec(self.x_, -self.y_, False)

	def __add__(self, rhs):
		if self.isZero():
			return rhs
		if rhs.isZero():
			return self
		if self.x_ == rhs.x_:
			# P + (-P) = 0
			if self.y_ == -rhs.y_:
				return Ec()
			# dbl
			L = self.x_ * self.x_
			L = (L + L + L + self.a_) / (self.y_ + self.y_)
		else:
			L = (self.y_ - rhs.y_) / (self.x_ - rhs.x_)
		x3 = L * L - (self.x_ + rhs.x_)
		y3 = L * (self.x_ - x3) - self.y_
		return Ec(x3, y3, False)

	def __mul__(self, rhs):
		if type(rhs) is Fr:
			rhs = rhs.v_
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
	assert (P * Ec.r_).isZero(), "order"
	a = 12345678932
	b = 98763445345
	aP = P * a
	bP = P * b
	assert aP * b == bP * a, "DH"

if __name__ == '__main__':
	main()
