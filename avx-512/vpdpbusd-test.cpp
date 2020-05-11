/*
	sample of vpdpbusd
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <xbyak/xbyak_util.h>

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

void vpdpbusdC(int *dst, const uint8_t *u, const int8_t *s)
{
	for (int i = 0; i < 16; i++) {
		int sum = dst[i];
		for (int j = 0; j < 4; j++) {
			sum += u[i * 4 + j] * s[i * 4 + j];
		}
		dst[i] = sum;
	}
}

int main()
	try
{
	Xbyak::util::Cpu cpu;
	if (!cpu.has(Xbyak::util::Cpu::tAVX512_VNNI)) {
		printf("AVX512_VNNI is not supported\n");
		return 1;
	}
	Code c;
	auto vpdpbusd = c.getCode<void (*)(int *dst, const uint8_t *u, const int8_t *s)>();
	int8_t s[64];
	uint8_t u[64];
	int dst1[16], dst2[16];
	for (int i = 0; i < 64; i++) {
		s[i] = int8_t(i - 32);
		u[i] = uint8_t(2 * i + 15);
		printf("(%-3d %3d) ", s[i], u[i]);
		if ((i % 8) == 7) puts("");
	}
	puts("");
	for (int i = 0; i < 16; i++) {
		dst1[i] = i + 9;
		dst2[i] = dst1[i];
	}
	vpdpbusd(dst1, u, s);
	vpdpbusdC(dst2, u, s);
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
