#include <xbyak_aarch64/xbyak_aarch64.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

using namespace Xbyak_aarch64;

struct Code : CodeGenerator {
	explicit Code()
		 : CodeGenerator(4096)
	{
		// func(dst, src, offset)
		const auto& dst = x0;
		const auto& src = x1;
		const auto& offset = x2;
		ptrue(p0.s);
		ld1w(z0.s, p0/T_z, ptr(offset));
		ld1w(z1.s, p0, ptr(src, z0.s, SXTW));
		st1w(z1.s, p0/T_z, ptr(dst));
		ret();
	}
};

int main()
	try
{
	Code c;
	c.ready();
	auto func = c.getCode<void (*)(uint32_t *, const void *, const void *)>();
	const size_t N = 16;
	uint32_t dst[N] = {};

	uint32_t src[N];
	uint32_t offset[N];
	for (size_t i = 0; i < N; i++) {
		src[i] = 0x01010101 * i;
	}
	for (size_t i = 0; i < N; i++) {
		offset[i] = i * 4;
	}
	func(dst, src, offset);
	for (size_t i = 0; i < N; i++) {
		printf("i=%zd %08x\n", i, dst[i]);
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
