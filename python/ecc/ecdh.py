from ec import Ec, Fp, Fr, initSecp256k1

def main():
	P = initSecp256k1()
	a = Fr()
	b = Fr()
	a.setRand()
	b.setRand()
	print(f"a={a}")
	print(f"b={b}")
	aP = P * a
	bP = P * b
	baP = aP * b
	abP = bP * a
	print(f"baP={baP}")
	print(f"abP={abP}")
	print(f"baP == abP? {baP == abP}")

if __name__ == '__main__':
	main()
