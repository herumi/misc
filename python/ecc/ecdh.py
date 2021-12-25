from ec import Ec, Fp, Fr, initSecp256k1

def main():
	P = initSecp256k1()
	a = Fr()
	b = Fr()
	a.setByCSPRNG()
	b.setByCSPRNG()
	print(f"a={a}")
	print(f"b={b}")
	aP = P.pow(a)
	bP = P.pow(b)
	baP = aP.pow(b)
	abP = bP.pow(a)
	print(f"baP={baP}")
	print(f"abP={abP}")
	print(f"baP == abP? {baP == abP}")

if __name__ == '__main__':
	main()
