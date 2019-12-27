#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/test.hpp>
#include <mcl/bls12_381.hpp>
#include <iostream>
#include <fstream>

using namespace mcl;
using namespace mcl::bn;

#define PUT(x) std::cout << #x "=" << (x) << std::endl;
#include "maptog2_wb19.hpp"

#if 0
void test_osswu2_help(const MapToG2_WB19& mapto)
{
	std::ifstream ifs("test.txt");
	for (int i = 0; i < 201; i++) {
		Fp2 t, x, y;
		ifs >> t;
		Fp2 x0, y0, z0;
		mapto.osswu2_help(x0, y0, z0, t);
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
#endif

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

template<class T>
void test1(const T& mapto)
{
	const struct {
		const char *ta;
		const char *tb;
		const char *xa;
		const char *xb;
		const char *ya;
		const char *yb;
		const char *za;
		const char *zb;
	} tbl[] = {
		{
			"0xe54bc0f2e26071a79ba5fe7ae5307d39cf5519e581e03b43f39a431eccc258fa1477c517b1268b22986601ee5caa5ea",
			"0x17e8397d5e687ff7f915c23f27fe1ca2c397a7df91de8c88dc82d34c9188a3ef719f9f20436ea8a5fe7d509fbc79214d",

			"0x11d568058220b1826cacde2e367beef98ea1edfde5fbf0491231b7ffdfc867e5269f9cfe65347c32ead182ba6b8c3ba1",
			"0x19f2778213e671ac444b1b579bfdf4e7fabeed9626dc909ce243b60397a6b5f65af0fbbe02a43c1e289f28c927012da1",

			"0xfe17bc695a84ec060b6287a4e77a50f65ba8f2c6c433f8131036ddfe34e3071d1cb71c0000f6bcfada947b19d8588df",
			"0xb76abd285945f787721e7e306895149523941586ac44f25a294c406a70ed570020992025aa307777cfe6c590567dfbe",

			"0x1910249ae63241608e013eb13578b9b3d96774d35e5732fc75efd17c212dd310d7f4016d6f212f62f33d34f10252e3e3",
			"0xdcd076cea67c76a6d0594c8f30c8cd8e9ead24f90870f723228f2203a55e04a5517c426ea2c4bae9d37a11c3d0f1912",
		},
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		Fp2 t(tbl[i].ta, tbl[i].tb);
		Fp2 x0(tbl[i].xa, tbl[i].xb);
		Fp2 y0(tbl[i].ya, tbl[i].yb);
		Fp2 z0(tbl[i].za, tbl[i].zb);
		Fp2 x, y, z;
		mapto.osswu2_help(x, y, z, t);
		CYBOZU_TEST_EQUAL(x, x0);
		CYBOZU_TEST_EQUAL(y, y0);
		CYBOZU_TEST_EQUAL(z, z0);
	}
}

CYBOZU_TEST_AUTO(test)
{
	MapToG2_WB19 mapto;
	mapto.init();
	test1(mapto);
}

#if 0
int main(int argc, char *argv[])
	try
{
	std::cout << std::hex;
	testSign(mapto);
//	test_osswu2_help(mapto);
	Fp2 t2("0x1d6cd21291f16108d65f33eaa90150796884328c921c15244c612cbb7d38bbefac341f2c8bb3ca9374d683e6fafa6af", "0xd306b469c9158cb9f9cf9898745909384b2bb1e3ed03734bd0f1ffd309d9da44679251f8f9e54e6d5775e6c713fd46");
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
#endif

int main(int argc, char *argv[])
{
	initPairing(mcl::BLS12_381);
	MapToG2_WB19 mapto;
	mapto.init();
	return cybozu::test::autoRun.run(argc, argv);
}
