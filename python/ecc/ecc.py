from field import Field as Fp

class Ec:
	a_ = Fp()
	b_ = Fp()
	n_ = 0

	@classmethod
	def init(cls, a, b, n):
		print(f"a={a} b={b}")
		cls.a_ = Fp(a)
		cls.b_ = Fp(b)
		cls.n_ = 0

	def __init__(self, x=None, y=None):
		self.isZero_ = True
		if x is None and y is None:
			return
		self.x_ = x
		self.y_ = y
		self.isZero_ = False
		if not self.isValid():
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
		ret = Ec()
		ret.x_ = self.x_
		ret.y_ = -self.y_
		ret.isZero_ = False
		return ret

	def __add__(self, rhs):
		if self.isZero():
			return rhs
		if rhs.isZero():
			return self
		if self.x_ == rhs.x_:
			if self.y_ == -rhs.y_:
				return Ec()
			L = self.x_ * self.x_
			L = (L + L + L + self.a_) / (self.y_ + self.y_)
		else:
			L = (self.y_ - rhs.y_) / (self.x_ - rhs.x_)
		x3 = L * L - (self.x_ + rhs.x_)
		y3 = L * (self.x_ - x3) - self.y_
		ret = Ec()
		ret.x_ = x3
		ret.y_ = y3
		ret.isZero_ = False
		return ret

	def pow(self, n):
		if n == 0:
			return Ec()
		bs = bin(n)[2:]
		ret = Ec()
		for b in bs:
			ret += ret
			if b == '1':
				ret += self
		return ret

def initSecp256k1():
	Fp.init(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f)

	a = 0
	b = 7
	n = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141
	Ec.init(a, b, n)
	x = Fp(0x79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798)
	y = Fp(0x483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8)
	P = Ec(x, y)
	return P

def main():
	P = initSecp256k1()
	Q = Ec()
	for i in range(200):
		R = P.pow(i)
		if Q != R:
			print(f"err i={i} Q={Q} R={R}")
		Q += P
	print(P.pow(Ec.n_).isZero())
	a = 12345678932
	b = 98763445345
	aP = P.pow(a)
	bP = P.pow(b)
	print(aP.pow(b))
	print(bP.pow(a))

if __name__ == '__main__':
	main()
