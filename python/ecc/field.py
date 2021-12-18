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

class Field:
	p_ = 1

	@classmethod
	def init(cls, p):
		cls.p_ = p

	def __init__(self, v_=0):
		self.v_ = v_ % Field.p_

	def __eq__(self, rhs):
		if type(rhs) is int:
			return self.v_ == rhs
		return self.v_ == rhs.v_

	def __str__(self):
		return str(self.v_)

	def __neg__(self):
		return Field(-self.v_)

	def __add__(self, rhs):
		return Field(self.v_ + rhs.v_)

	def __sub__(self, rhs):
		return Field(self.v_ - rhs.v_)

	def __mul__(self, rhs):
		return Field(self.v_ * rhs.v_)

	def __truediv__(self, rhs):
		return self * rhs.inv()

	def inv(self):
		return Field(invMod(self.v_, Field.p_))


def main():
	p = 17
	Field.init(p)
	a = Field(5)
	print(a)
	b = Field(-5)
	print(b)
	print(a + b)
	for v in range(1, p):
		x = Field(v)
		r = x.inv()
		print(x, r, x * r, (x * r) / r)

if __name__ == '__main__':
	main()
