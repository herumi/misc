#include <mcl/bls12_381.hpp>
#include <iostream>
#include <fstream>

using namespace mcl;
using namespace mcl::bn;

#define PUT(x) std::cout << #x "=" << (x) << std::endl;
#include "maptog2.hpp"

void test_osswu2_help(const MapToG2onBLS12_381& mapto)
{
	std::ifstream ifs("osswu2_help.txt");
	Fp2 t, x, y;
	int i = 0;
	while (ifs >> t) {
		Fp2 x0, y0;
		mapto.osswu2_help(x0, y0, t);
		ifs >> x >> y;
		if (x != x0) {
			printf("%3d err\nx=%s\nx0=%s\n", i, x.getStr(16).c_str(), x0.getStr(16).c_str());
			exit(1);
		}
		if (y != y0) {
			printf("%3d err\ny=%s\ny0=%s\n", i, y.getStr(16).c_str(), y0.getStr(16).c_str());
			printf("-y=%s\n", (-y0).getStr(16).c_str());
			exit(1);
		}
		i++;
	}
}

int main()
	try
{
	std::cout << std::hex;
	initPairing(mcl::BLS12_381);
	MapToG2onBLS12_381 mapto;
	mapto.init();
	test_osswu2_help(mapto);
	Fp2 t1("0xbd85cfa89e7d8787399c27fe122f7b7bd8e16c7baed07ad130fa50705bd05d022ac5d150e1db578eb5b032385e25da", "0x19aca2fa3ab7b207c28d99fe4c2384084e0cd7c61cf4389df4e351eaf10fb55430a24cfc732f4550dc9209a68c11661");
	Fp2 t2("0xf6721953dbe192e1502bfe429b610f5b5662df0f6b747eee2f622086e1cd5838bf506edde08489d51d68ad3ebcfe0d", "0xce2ae5b302d9f167ddf643b5893d52385ab1db46fa708d10fdd7cb504b700bfaa38b17b461848500734cdf5dea12f0f");
	Fp2 x, y;
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
