#include <xbyak_aarch64/xbyak_aarch64.h>
#include <functional>
#include "fexpa.hpp"

using namespace Xbyak;

struct Code : CodeGenerator {
	Xbyak::Label g_dataL;
	Code(int mode)
	{
		const auto& dst = x0;
		const auto& src = x1;
		ptrue(p0.s);
		if ((mode & 1) == 0) {
			puts("ld1w + ld1w");
			ld1w(z0.s, p0/T_z, ptr(src));
			ld1w(z1.s, p0/T_z, ptr(src, 1));
		} else {
			puts("ld2w");
			ld2w(z0.s, p0/T_z, ptr(src));
		}
		if ((mode & 2) == 0) {
			puts("st1w + st1w");
			st1w(z0.s, p0/T_z, ptr(dst));
			st1w(z1.s, p0/T_z, ptr(dst, 1));
		} else {
			puts("st2w");
			st2w(z0.s, p0/T_z, ptr(dst));
		}
		ret();
	}
};

bool detect(const float *x, const float *y, size_t i, int mode)
{
	float v = 0;
	switch (mode) {
	case 0:
	case 3:
		v = x[i];
		break;
	case 1:
		v = (i < 16) ? x[i * 2] : x[i * 2 + 1 - 32];
		break;
	case 2:
		v = ((i & 1) == 0) ? x[i / 2] : x[i / 2 + 16];
		break;
	}
	return y[i] == v;
}

int main()
	try
{
	const size_t N = 32;
	float x[N], y[N];
	for (int mode = 0; mode < 4; mode++) {
		printf("mode=%d\n", mode);
		for (size_t i = 0; i < N; i++) {
			x[i] = -(i * 0.123) + 0.4;
			y[i] = -99;
		}
		Code c(mode);
		auto f = c.getCode<void (*)(float *, const float *)>();
		c.ready();
		f(y, x);
		for (size_t i = 0; i < N; i++) {
			printf("%2zd x=%f y=%f %c\n", i, x[i], y[i], detect(x, y, i, mode) ? 'o' : 'x');
		}
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
