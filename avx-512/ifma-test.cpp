#include <xbyak/xbyak_util.h>
#include "util.hpp"

using namespace Xbyak::util;

int main()
	try
{
	Xbyak::util::Cpu cpu;
	if (!cpu.has(Xbyak::util::Cpu::tAVX512_IFMA)) {
		printf("AVX512_IFMA is not supported\n");
		return 1;
	}
	const int N = 8;
	const uint64_t _a[N] = {
		0x12345678abcdefab, 0x45678abcdefab,
	};
	const uint64_t _b[N] = {
		0xffeeddccbbaa8877, 0xeddccbbaa8877,
	};
	const uint64_t _c[N] = {
		0xffffaabbccddeeff, 0xffffaabbccddeeff,
	};
	Vec a, b, c, z;
	set(a, _a);
	set(b, _b);
	set(c, _c);
	z = vmulL(a, b, c); // low(c+a*b)
	dump(a, "a");
	dump(b, "b");
	dump(c, "c");
	dump(z, "z");
	const uint64_t mask52 = getMask(52);
	uint128_t ia = _a[0];
	uint128_t ib = _b[0];
	uint128_t ic = _c[0];
	uint64_t iz = (mask52 & ((mask52 & ia) * (mask52 & ib))) + ic;
	dump(iz, "i");
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}
