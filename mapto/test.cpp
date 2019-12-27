#include <mcl/bls12_381.hpp>
#include <iostream>
#include <fstream>

using namespace mcl;
using namespace mcl::bn;

#define PUT(x) std::cout << #x "=" << (x) << std::endl;
#include "maptog2_wb19.hpp"

void test_osswu2_help(const MapToG2_WB19& mapto)
{
	std::ifstream ifs("test.txt");
	for (int i = 0; i < 201; i++) {
		Fp2 t, x, y;
		ifs >> t;
		Fp2 x0, y0;
		mapto.osswu2_help(x0, y0, t);
		ifs >> x >> y;
		if (x != x0) {
			printf("%3d err\nx=%s\nx0=%s\n", i, x.getStr(16).c_str(), x0.getStr(16).c_str());
			exit(1);
		}
		if (y != y0) {
			printf("%3d err\ny=%s\ny0=%s\n", i, y.getStr(16).c_str(), y0.getStr(16).c_str());
			exit(1);
		}
	}
	puts("ok");
}

template<class T>
void testSign(const T& mapto)
{
	const Fp& H = mapto.half;
	const size_t N = 4;
	const Fp tbl[N] = { 0, 1, H, H + 1 };
	const int expect[N][N] = {
		{  1, 1, 1, -1 },
		{  1, 1, 1, -1 },
		{  1, 1, 1, -1 },
		{ -1, 1, 1, -1 },
	};
	Fp2 t;
	for (size_t i = 0; i < N; i++) {
		t.a = tbl[i];
		for (size_t j = 0; j < N; j++) {
			t.b = tbl[j];
			if (mapto.isNegSign(t) != (expect[i][j] < 0)) {
				printf("err %zd %zd\n", i, j);
			}
		}
	}
	puts("ok");
}

int main()
	try
{
	std::cout << std::hex;
	initPairing(mcl::BLS12_381);
	MapToG2_WB19 mapto;
	mapto.init();
	testSign(mapto);
//	test_osswu2_help(mapto);
	Fp2 t1("0xe54bc0f2e26071a79ba5fe7ae5307d39cf5519e581e03b43f39a431eccc258fa1477c517b1268b22986601ee5caa5ea", "0x17e8397d5e687ff7f915c23f27fe1ca2c397a7df91de8c88dc82d34c9188a3ef719f9f20436ea8a5fe7d509fbc79214d");
	Fp2 t2("0x1d6cd21291f16108d65f33eaa90150796884328c921c15244c612cbb7d38bbefac341f2c8bb3ca9374d683e6fafa6af", "0xd306b469c9158cb9f9cf9898745909384b2bb1e3ed03734bd0f1ffd309d9da44679251f8f9e54e6d5775e6c713fd46");
	Fp2 x, y;
	mapto.osswu2_help(x, y, t1);
	PUT(x);
	PUT(y);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
