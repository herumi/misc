#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak;

void intAdd(int *z, const int *x, const int *y, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		z[i] = x[i] + y[i];
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
		ld1w(z0.s, p0/T_z, ptr(src1, x4, LSL, 2));
		ld1w(z1.s, p0/T_z, ptr(src2, x4, LSL, 2));
		add(z0.s, z0.s, z1.s);
		st1w(z0.s, p0, ptr(out, x4, LSL, 2));
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
	auto f = c.getCode<void (*)(int *, const int *, const int *, size_t)>();
	c.ready();
	const size_t n = 20;
	int x[n], y[n], z1[n], z2[n];
	for (size_t i = 0; i < n; i++) {
		x[i] = i;
		y[i] = i;
		z1[i] = -99;
		z2[i] = z1[i];
	}
	size_t nn = 17;
	intAdd(z1, x, y, nn);
	f(z2, x, y, nn);
	for (size_t i = 0; i < n; i++) {
		printf("x=%d y=%d z1=%d z2=%d %c\n", x[i], y[i], z1[i], z2[i], (z1[i] == z2[i]) ? 'o' : 'x');
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
