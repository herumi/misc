#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak;

void sqrAdd(double *z, const double *x, const double *y, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		z[i] = x[i] * x[i] + y[i];
	}
}

struct Code : CodeGenerator {
	void genScalar()
	{
		Label check;
		const auto& out = x0;
		const auto& src1 = x1;
		const auto& src2 = x2;
		const auto& n = x3;
		mov(x4, 0);
		b(check);
	Label lp = L();
		ldr(d1, ptr(src1, x4, LSL, 3));
		ldr(d2, ptr(src2, x4, LSL, 3));
		fmadd(d2, d1, d1, d2);
		str(d2, ptr(out, x4, LSL, 3));
		add(x4, x4, 1);
	L(check);
		cmp(x4, n);
		blt(lp);
		ret();
	}
	void genSVE()
	{
		const auto& out = x0;
		const auto& src1 = x1;
		const auto& src2 = x2;
		const auto& n = x3;
		mov(x4, 0);
		whilelt(p0.d, x4, n);
	Label lp = L();
		ld1d(z1.d, p0/T_z, ptr(src1, x4, LSL, 3));
		ld1d(z2.d, p0/T_z, ptr(src2, x4, LSL, 3));
		fmla(z2.d, p0/T_m, z1.d, z1.d);
		st1d(z2.d, p0, ptr(out, x4, LSL, 3));
		incd(x4);
		whilelt(p0.d, x4, n);
		b_first(lp);
		ret();
	}
};

int main()
	try
{
	Code c;
	auto f1 = c.getCurr<void (*)(double *, const double *, const double *, size_t)>();
	c.genScalar();
	printf("f1=%p\n", f1);
	c.align(16);
	auto f2 = c.getCurr<void (*)(double *, const double *, const double *, size_t)>();
	c.genSVE();
	c.ready();
	printf("f2=%p\n", f2);
	const size_t n = 20;
	double x[n], y[n], z1[n], z2[n], z3[n];
	for (size_t i = 0; i < n; i++) {
		x[i] = i;
		y[i] = i;
		z1[i] = -99;
		z2[i] = z1[i];
		z3[i] = z1[i];
	}
	size_t nn = 17;
	f1(z1, x, y, nn);
	f2(z2, x, y, nn);
	sqrAdd(z3, x, y, nn);
	for (size_t i = 0; i < n; i++) {
		printf("x=%f y=%f z1=%f z2=%f z3=%f %c\n", x[i], y[i], z1[i], z2[i], z3[i], (z1[i] == z2[i] && z2[i] == z3[i]) ? 'o' : 'x');
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
