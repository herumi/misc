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

class Fr:
	p_ = 1

	@classmethod
	def init(cls, p):
		cls.p_ = p

	def setRand(self):
		self.v_ = secrets.randbelow(self.p_)

	def __init__(self, v_=0):
		self.v_ = v_ % Fr.p_

	def __eq__(self, rhs):
		if type(rhs) is int:
			return self.v_ == rhs
		return self.v_ == rhs.v_

	def __str__(self):
		return str(self.v_)

	def __neg__(self):
		return Fr(-self.v_)

	def __add__(self, rhs):
		return Fr(self.v_ + rhs.v_)

	def __sub__(self, rhs):
		return Fr(self.v_ - rhs.v_)

	def __mul__(self, rhs):
		return Fr(self.v_ * rhs.v_)

	def __truediv__(self, rhs):
		return self * rhs.inv()

	def inv(self):
		# v * v^(p-2) = v^(p-1) = 1 mod p
		if self.v_ == 0:
			raise Exception("zero inv")
		return Fr(pow(self.v_, Fr.p_ - 2, Fr.p_))
#		return Fr(invMod(self.v_, Fr.p_))

def main():
	p = 43
	Fr.init(p)
	m1 = 8
	m2 = 17
	assert Fr(m1).v_ == m1, "eq1"
	assert Fr(-m1).v_ == (-m1) % Fr.p_, "eq2"
	assert Fr(m1 * m2).v_ == (m1 * m2) % Fr.p_, "eq3"
	assert Fr(m1) + Fr(m2) == Fr(m1 + m2), "add"
	assert Fr(m1) - Fr(m2) == Fr(m1 - m2), "sub"
	assert Fr(m1) * Fr(m2) == Fr(m1 * m2), "mul"
	assert -Fr(m1) == Fr(-m1), "minus"
	for v in range(1, Fr.p_):
		x = Fr(v)
		r = x.inv()
		assert (x * r) / r == x, "inv"

if __name__ == '__main__':
	main()
