import secrets
# return (gcd, x, y) such that gcd = a * x + b * y
def extGcd(a, b):
	if a == 0:
		return b, 0, 1
	q, r = divmod(b, a)
	gcd, x, y = extGcd(r, a)
	return gcd, y - q * x, x

# return rev such that (r * x) mod p = 1
def invMod(x, p):
	gcd, rev, _ = extGcd(x, p)
	if gcd != 1:
		raise Exception("invMod", x, p, gcd, rev)
	if rev < p:
		rev += p
	return rev

class Fp:
	p = 1

	@classmethod
	def init(cls, p):
		cls.p = p

	def setRand(self):
		self.v = secrets.randbelow(self.p)

	def __init__(self, v=0):
		self.v = v % Fp.p

	def __eq__(self, rhs):
		if type(rhs) is int:
			return self.v == rhs
		return self.v == rhs.v

	def __str__(self):
		return str(self.v)

	def __neg__(self):
		return Fp(-self.v)

	def __add__(self, rhs):
		return Fp(self.v + rhs.v)

	def __sub__(self, rhs):
		return Fp(self.v - rhs.v)

	def __mul__(self, rhs):
		return Fp(self.v * rhs.v)

	def __truediv__(self, rhs):
		return self * rhs.inv()

	def inv(self):
		# v * v^(p-2) = v^(p-1) = 1 mod p
		if self.v == 0:
			raise Exception("zero inv")
		return Fp(pow(self.v, Fp.p - 2, Fp.p))
#		return Fp(invMod(self.v, Fr.p))

def main():
	p = 43
	Fp.init(p)
	m1 = 8
	m2 = 17
	assert Fp(m1).v == m1, "eq1"
	assert Fp(-m1).v == (-m1) % Fp.p, "eq2"
	assert Fp(m1 * m2).v == (m1 * m2) % Fp.p, "eq3"
	assert Fp(m1) + Fp(m2) == Fp(m1 + m2), "add"
	assert Fp(m1) - Fp(m2) == Fp(m1 - m2), "sub"
	assert Fp(m1) * Fp(m2) == Fp(m1 * m2), "mul"
	assert -Fp(m1) == Fp(-m1), "minus"
	for v in range(1, Fp.p):
		x = Fp(v)
		r = x.inv()
		assert (x * r) / r == x, "inv"

if __name__ == '__main__':
	main()
