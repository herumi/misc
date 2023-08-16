template<int N>
void test(const char *Mstr)
{
	InvModT<N> invMod;
	invMod.init(Mstr);
	std::cout << "M " << invMod.mM << std::endl;
	printf("Mi %ld\n", invMod.Mi);

	const int C = 10000;
	mpz_class x, y, z;
	x = 1;
	for (int i = 0; i < C; i++) {
		mcl::gmp::invMod(y, x, invMod.mM);
		invMod.inv(z, x);
		CYBOZU_TEST_EQUAL(y, z);
		x = y + 1;
	}
	for (x = 1; x < 1000; x++) {
		mcl::gmp::invMod(y, x, invMod.mM);
		invMod.inv(z, x);
		CYBOZU_TEST_EQUAL(y, z);
	}
	x = invMod.mM - 1;
	for (int i = 0; i < C; i++) {
		mcl::gmp::invMod(y, x, invMod.mM);
		invMod.inv(z, x);
		CYBOZU_TEST_EQUAL(y, z);
		x--;
	}
	CYBOZU_BENCH_C("modinv", 1000, x++;invMod.inv, x, x);
}

CYBOZU_TEST_AUTO(mini)
{
	const char *Mstr = "fb"; // 251
	InvModT<1> invMod;
	invMod.init(Mstr);
	std::cout << "M " << invMod.mM << std::endl;
	printf("Mi %ld\n", invMod.Mi);

	mpz_class x, y, z;
	for (int i = 1; i < 251; i++) {
		x = i;
		mcl::gmp::invMod(y, x, invMod.mM);
		invMod.inv(z, x);
		CYBOZU_TEST_EQUAL(y, z);
	}
}

CYBOZU_TEST_AUTO(main)
{
	const char *tbl6[] = {
		"1a0111ea397fe69a4b1ba7b6434bacd764774b84f38512bf6730d2a0f6b0f6241eabfffeb153ffffb9feffffffffaaab",
	};
	const char *tbl5[] = {
		"fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f",
	};
	const char *tbl4[] = {
		"73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001",
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl6); i++) {
		test<6>(tbl6[i]);
	}
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl5); i++) {
		test<5>(tbl5[i]);
	}
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl4); i++) {
		test<4>(tbl4[i]);
	}
}
