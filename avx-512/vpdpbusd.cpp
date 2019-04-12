#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#include <cybozu/xorshift.hpp>

/*
	512bit = 64byte = dword * 16
*/

using namespace Xbyak::util;

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		StackFrame sf(this, 3);
		vmovdqu32(zm0, ptr[sf.p[0]]);
		vmovdqu8(zm1, ptr[sf.p[1]]);
		vpdpbusd(zm0, zm1, ptr[sf.p[2]]);
		vmovdqu32(ptr[sf.p[0]], zm0);
		vzeroupper();
		ret();
	}
};

void put(const int *x)
{
	for (int i = 0; i < 16; i++) {
		printf("% 5d ", x[i]);
		if (i == 7) puts("");
	}
	puts("");
}

void dot_prod(int *dst, const uint8_t *u, const int8_t *s)
{
	for (int i = 0; i < 16; i++) {
		int sum = dst[i];
		for (int j = 0; j < 4; j++) {
			sum += s[i * 4 + j] * u[i * 4 + j];
		}
		dst[i] = sum;
	}
}

int main()
	try
{
	Code c;
	auto f = c.getCode<void (*)(int *dst, const uint8_t *u, const int8_t *s)>();
	cybozu::XorShift rg;
	int8_t s[64];
	uint8_t u[64];
	int dst1[16], dst2[16];
	for (int i = 0; i < 64; i++) {
		s[i] = int8_t(rg());
		u[i] = uint8_t(rg());
		printf("(%-3d %3d) ", s[i], u[i]);
		if ((i % 8) == 7) puts("");
	}
	puts("");
	for (int i = 0; i < 16; i++) {
		dst1[i] = i + 9;
		dst2[i] = dst1[i];
	}
	f(dst1, u, s);
	dot_prod(dst2, u, s);
	puts("asm");
	put(dst1);
	puts("C");
	put(dst2);
	bool err = false;
	for (int i = 0; i < 16; i++) {
		if (dst1[i] != dst2[i]) {
			printf("ERR %d %d %d\n", i, dst1[i], dst2[i]);
			err = true;
		}
	}
	if (!err) puts("ok");
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}

