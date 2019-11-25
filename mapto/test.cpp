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
	for (int i = 0; i < 200; i++) {
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
}

int main()
	try
{
	std::cout << std::hex;
	initPairing(mcl::BLS12_381);
	MapToG2_WB19 mapto;
	mapto.init();
	test_osswu2_help(mapto);
	Fp2 t1("0xbd85cfa89e7d8787399c27fe122f7b7bd8e16c7baed07ad130fa50705bd05d022ac5d150e1db578eb5b032385e25da", "0x19aca2fa3ab7b207c28d99fe4c2384084e0cd7c61cf4389df4e351eaf10fb55430a24cfc732f4550dc9209a68c11661");
	Fp2 t2("0xf6721953dbe192e1502bfe429b610f5b5662df0f6b747eee2f622086e1cd5838bf506edde08489d51d68ad3ebcfe0d", "0xce2ae5b302d9f167ddf643b5893d52385ab1db46fa708d10fdd7cb504b700bfaa38b17b461848500734cdf5dea12f0f");
	t1.setStr("0x97ed293a1a6e47ec799abcd6c727f1c5ed4a23bb349bc5a24ed1ced25a3c793e098729a5957ee20fff4f9f9be6f313e 0x5883fdf9394f4502abdda48cac49ab386fbbd7b11df466b75563a961ddb398d525333b2b3fd3ed470926e2e2312e095");
	Fp2 x, y;
	mapto.osswu2_help(x, y, t1);
	PUT(x);
	PUT(y);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
