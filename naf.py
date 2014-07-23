def convNaf(x):
	n3 = bin(x * 3)[2:]
	n1 = bin(x)[2:]
	padding = len(n3) - len(n1)
	if padding:
		n1 = '0' * padding + n1
	print n3
	print n1
	ret = []
	for i in xrange(len(n3)):
		ret.append(ord(n3[i]) - ord(n1[i]))
	return ret[:-1]

def getVal(v):
	z = 0
	n = len(v)
	for i in xrange(n):
		z += (1 << i) * v[n - 1 - i]
	return z


z = 4965661367192848881
print z
v = convNaf(z)
print v
print getVal(v)
