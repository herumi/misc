#include <xbyak_aarch64/xbyak_aarch64.h>
#include <functional>
#include "fexpa.hpp"
#include "floatformat.hpp"

using namespace Xbyak_aarch64;

struct Code : CodeGenerator {
	explicit Code(int mode)
		 : CodeGenerator(4096)
	{
		const auto& psrc1 = x1;
		const auto& psrc2 = x2;
		const auto& src1 = z0;
		const auto& src2 = z1;
		ptrue(p0.s);
		ld1w(src1.s, p0/T_z, ptr(psrc1));
		ld1w(src2.s, p0/T_z, ptr(psrc2));
		gen(mode, src1, src2);
		str(p1, ptr(x0));
		ret();
	}
	void gen(int mode, const ZReg& src1, const ZReg& src2)
	{
		switch (mode) {
		case 0:
			puts("pture(p1.b)");
			ptrue(p1.b);
			break;
		case 1:
			puts("pture(p1.s)");
			ptrue(p1.s);
			break;
		case 2:
			puts("pture(p1.d)");
			ptrue(p1.d);
			break;
		case 3:
			puts("cmple");
			cmple(p1.s, p0.s, src1.s, src2.s);
			break;
		default:
			printf("bad mode=%d\n", mode);
			exit(1);
		}
	}
};

int main(int argc, char *argv[])
	try
{

	int mode = argc == 1 ? 0 : atoi(argv[1]);
	printf("mode=%d\n", mode);
	Code c(mode);
	c.ready();
	auto func = c.getCode<void (*)(uint64_t *, const float *, const float *)>();
	const size_t N = 16;
	float src1[N] = {};
	float src2[N] = {};
	uint64_t pred;

	const struct {
		uint32_t s;
		uint32_t e;
		uint32_t f;
	} tbl[] = {
		{ 0, 0, 0 },
		{ 0, 0, 1 },
		{ 0, 127, 0 },
		{ 0, 128, 123 },
		{ 0, 255, 0 },
		{ 0, 255, 1 },
		{ 0, 255, 1 << 22 },
		{ 0, 255, 1 << 21 },
	};
	for (size_t i = 0; i < N / 2; i++) {
		FloatFormat ff;
		uint32_t s = tbl[i].s;
		uint32_t e = tbl[i].e;
		uint32_t f = tbl[i].f;
		ff.set(s, e, f);
		src1[i * 2 + 0] = ff.get().f;
		ff.set(1 - s, e, f);
		src1[i * 2 + 1] = ff.get().f;
		src2[i * 2 + 0] = 1.0f;
		src2[i * 2 + 1] = -1.0f;
	}
	func(&pred, src1, src2);
	for (size_t i = 0; i < N; i++) {
		fi fi;
		fi.f = src1[i];
		printf("%e(%08x) ", fi.f, fi.i);
		fi.f = src2[i];
		printf("%e(%08x)\n", fi.f, fi.i);
	}
	puts("->");
	for (int i = 63; i >= 0; i--) {
		printf("%d", int((pred >> i) & 1));
		if ((i & 3) == 0) printf(":");
	}
	printf("\n");
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
