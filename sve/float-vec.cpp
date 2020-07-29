#include <xbyak_aarch64/xbyak_aarch64.h>
#include <cybozu/benchmark.hpp>

using namespace Xbyak;

void sqrAdd(float *z, const float *x, const float *y, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		z[i] = x[i] * x[i] + y[i];
	}
}

struct Code : CodeGenerator {
	explicit Code(int N = 0)
	{
		typedef Xbyak::ZReg ZReg;
		const auto& out = x0;
		const auto& src1 = x1;
		const auto& src2 = x2;
		const auto& n = x3;
		Label cond;
		if (0 < N && N <= 4) { // max z0, ..., z7
			ptrue(p0.s);
			Label skip;
			b(skip);
		Label lp = L();
			for (int i = 0; i < N; i++) ld1w(ZReg(i * 2).s, p0/T_z, ptr(src1, i));
			add(src1, src1, 64 * N);
			for (int i = 0; i < N; i++) ld1w(ZReg(i * 2 + 1).s, p0/T_z, ptr(src2, i));
			add(src2, src2, 64 * N);
			for (int i = 0; i < N; i++) fmla(ZReg(i * 2 + 1).s, p0/T_m, ZReg(i * 2).s, ZReg(i * 2).s);
			for (int i = 0; i < N; i++) st1w(ZReg(i * 2 + 1).s, p0, ptr(out, i));
			add(out, out, 64 * N);
			sub(n, n, 16 * N);
		L(skip);
			cmp(n, 16 * N);
			bge(lp);
		}
		mov(x4, 0);
		b(cond);
	Label remain = L();
		ld1w(z0.s, p0/T_z, ptr(src1, x4, LSL, 2));
		ld1w(z1.s, p0/T_z, ptr(src2, x4, LSL, 2));
		fmla(z1.s, p0/T_m, z0.s, z0.s);
		st1w(z1.s, p0, ptr(out, x4, LSL, 2));
		incd(x4);
	L(cond);
		whilelt(p0.s, x4, n);
		b_first(remain);
		ret();
	}
};

int main()
	try
{
	for (int N = 0; N <= 4; N++) {
		printf("N=%d\n", N);
		Code c(N);
		auto f = c.getCode<void (*)(float *, const float *, const float *, size_t)>();
		c.ready();
		const size_t n = 1024;
		float x[n], y[n], z1[n], z2[n];
		for (size_t i = 0; i < n; i++) {
			x[i] = i;
			y[i] = i;
			z1[i] = -99;
			z2[i] = z1[i];
		}
		size_t nn = 1024;
		sqrAdd(z1, x, y, nn);
		f(z2, x, y, nn);
		for (size_t i = 0; i < n; i++) {
			if (z1[i] != z2[i]) {
				printf("i=%zd x=%f y=%f z1=%f z2=%f\n", i, x[i], y[i], z1[i], z2[i]);
				return 1;
			}
		}
		puts("ok");
		CYBOZU_BENCH_C("f", 1000, f, z2, x, y, nn);
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
