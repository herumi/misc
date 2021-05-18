#include <xbyak_aarch64/xbyak_aarch64.h>
#include <functional>
#include "fexpa.hpp"
#include "floatformat.hpp"

using namespace Xbyak_aarch64;

float u2f(uint32_t x)
{
	fi fi;
	fi.i = x;
	return fi.f;
}

uint32_t f2u(float x)
{
	fi fi;
	fi.f = x;
	return fi.i;
}

struct Code : CodeGenerator {
	Label g_dataL;
	explicit Code(int mode)
		 : CodeGenerator(4096)
	{
		const auto& pdst = x0;
		const auto& psrc1 = x1;
		const auto& psrc2 = x2;
		const auto& dst = z0;
		const auto& src1 = z1;
		const auto& src2 = z2;
		ptrue(p0.s);
		ld1w(src1.s, p0/T_z, ptr(psrc1));
		ld1w(src2.s, p0/T_z, ptr(psrc2));
		gen(mode, dst, src1, src2);
		st1w(dst.s, p0/T_z, ptr(pdst));
		ret();
	}
	void gen(int mode, const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		switch (mode) {
		case 0:
			puts("fadd");
			fadd(dst.s, src1.s, src2.s);
			break;
		case 1:
			puts("lsr + fexpa");
			lsr(src1.s, src1.s, 17);
			fexpa(dst.s, src1.s);
			break;
		case 2:
			puts("fmin");
			movprfx(dst.s, p0, src1.s);
			fmin(dst.s, p0, src2.s);
			break;
		case 3:
			puts("fmax");
			movprfx(dst.s, p0, src1.s);
			fmax(dst.s, p0, src2.s);
			break;
		case 4:
			puts("frintm");
			frintm(dst.s, p0, src1.s);
			break;
		case 5:
			puts("fcvtzs");
			fcvtzs(dst.s, p0, src1.s);
			break;
		case 6:
			puts("fcpy");
			fcpy(dst.s, p0, 0);
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
	auto func = c.getCode<void (*)(float *, const float *, const float *)>();
	const size_t N = 16;
	float src1[N] = {};
	float src2[N] = {};
	float dst[N];

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
	func(dst, src1, src2);
	for (size_t i = 0; i < N; i++) {
		fi fi;
		fi.f = src1[i];
		printf("%e(%08x) %e ->", fi.f, fi.i, src2[i]);
		fi.f = dst[i];
		printf("%e(%08x)\n", fi.f, fi.i);
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
