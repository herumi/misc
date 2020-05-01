#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak;

void sqrAdd(float *z, const float *x, const float *y, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		z[i] = x[i] * x[i] + y[i];
	}
}

struct Code : CodeGenerator {
	Code()
	{
		const auto& out = x0;
		const auto& src1 = x1;
		const auto& src2 = x2;
		const auto& n = x3;
		Label cond;
		mov(x4, 0);
		b(cond);
	Label lp = L();
		ld1w(z1.s, p0/T_z, ptr(src1, x4, LSL, 2));
		ld1w(z2.s, p0/T_z, ptr(src2, x4, LSL, 2));
		fmla(z2.s, p0/T_m, z1.s, z1.s);
		st1w(z2.s, p0, ptr(out, x4, LSL, 2));
		incd(x4);
	L(cond);
		whilelt(p0.s, x4, n);
		b_first(lp);
		ret();
	}
};

int main()
	try
{
	Code c;
	auto f = c.getCode<void (*)(float *, const float *, const float *, size_t)>();
	c.ready();
	const size_t n = 20;
	float x[n], y[n], z1[n], z2[n];
	for (size_t i = 0; i < n; i++) {
		x[i] = i;
		y[i] = i;
		z1[i] = -99;
		z2[i] = z1[i];
	}
	size_t nn = 17;
	sqrAdd(z1, x, y, nn);
	f(z2, x, y, nn);
	for (size_t i = 0; i < n; i++) {
		printf("x=%f y=%f z1=%f z2=%f %c\n", x[i], y[i], z1[i], z2[i], (z1[i] == z2[i]) ? 'o' : 'x');
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
