
int main()
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
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl4); i++) {
		test<4>(tbl4[i]);
	}
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl5); i++) {
		test<5>(tbl5[i]);
	}
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl6); i++) {
		test<6>(tbl6[i]);
	}
}
