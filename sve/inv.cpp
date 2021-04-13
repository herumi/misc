#include <xbyak_aarch64/xbyak_aarch64.h>
#include <cybozu/benchmark.hpp>
#include <math.h>
/*
	clang -O3
float x[1024]
            inv.exe 0: inv.exe 1
            (frintm) : (fadd)
3.208usec ; 100  clk ; 100  clk ; fdiv
1.058usec ;  33  clk ;   7.9clk ; frintm and frecps use same z2
0.350usec ;  10.9clk ;   7.9clk ; frintm and frecps use diff regs
1.651usec ;  51  clk ;  11.0clk ; frintm and frecps x 2 use same z2
0.370usec ;  11.2clk ;  11.0clk ; frintm and frecps x 2 use diff reg
*/

using namespace Xbyak_aarch64;

struct Code : CodeGenerator {
	Code(int op, int mode)
	{
		const auto& out = x0;
		const auto& src1 = x1;
		const auto& src2 = x2;
		const auto& n = x3;
		const auto& idx = x4;
		const auto& one = z3;
		ptrue(p0.s);
		fcpy(one.s, p0, 1.0);
		mov(idx, 0);
		// assume n > 0 && (n % 16) == 0
	Label lp = L();
		ld1w(z0.s, p0/T_z, ptr(src1, idx, LSL, 2));
		ld1w(z1.s, p0/T_z, ptr(src2, idx, LSL, 2));
		switch (op) {
		case 0:
//movprfx(z2.s, p0/T_z, z0.s);
			frintm(z2.s, p0, z0.s); // floor
			break;
		case 1:
			fadd(z2.s, z0.s, z0.s);
			break;
		}
		fadd(z0.s, z1.s, z2.s);
		switch (mode) {
		case 0:
			fdivr(z0.s, p0, one.s);
			break;
		case 1:
			frecpe(z1.s, z0.s);
			frecps(z2.s, z0.s, z1.s);
			fmul(z0.s, z1.s, z2.s);
			break;
		case 2:
			frecpe(z1.s, z0.s);
			frecps(z3.s, z0.s, z1.s);
			fmul(z0.s, z1.s, z3.s);
			break;
		case 3:
			frecpe(z1.s, z0.s);
			frecps(z2.s, z0.s, z1.s);
			fmul(z1.s, z1.s, z2.s);

			frecps(z2.s, z0.s, z1.s);
			fmul(z0.s, z1.s, z2.s);
			break;
		case 4: // replace z2 with z3
			frecpe(z1.s, z0.s);
			frecps(z3.s, z0.s, z1.s);
			fmul(z1.s, z1.s, z3.s);

			frecps(z3.s, z0.s, z1.s);
			fmul(z0.s, z1.s, z3.s);
			break;
		case 5:
		/*
			y = approx(1/x)
			y = 1/x + e
			d = 1 - x y = 1 - x(1/x+e) = -xe
			d = d d + d = d(1+d) = -xe(1-xe)
			d = d y + y = y(1+d) = (1/x+e)(1-xe+(xe)^2)
			  = 1/x + x^2 e^3
			fmls(a, b, c) : a = a - b * c
			fmad(a, b, c) ; a = a * b + c
		*/
			frecpe(z1.s, z0.s);
			// d = 1 - x * y
			movprfx(z0.s, p0, one.s);
			fmls(z0.s, p0, z0.s, z1.s);
			// d = d * d + d
			fmad(z0.s, p0, z0.s, z0.s);
			// d = d * y + y
			fmad(z0.s, p0, z1.s, z1.s);
			break;
		}
		st1w(z0.s, p0, ptr(out, idx, LSL, 2));
		add(idx, idx, 16);
		cmp(idx, n);
		blt(lp);
		ret();
	}
};

void fC(int op, float *z, const float *x, const float *y, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		float v;
		switch (op) {
		case 0:
			v = floor(x[i]) + y[i];
			break;
		case 1:
			v = (x[i] + x[i]) + y[i];
			break;
		default:
			printf("ERR op=%d\n", op);
			exit(1);
		}
		z[i] = 1 / v;
	}
}

int main(int argc, char *argv[])
	try
{
	int op = argc == 1 ? 0 : atoi(argv[1]);
	const char *opTbl[] = {
		"floor",
		"add",
	};
	const char *modeTbl[] = {
		"fdivr",
		"frecps z2",
		"frecps z3",
		"frecpsx2 z2",
		"frecpsx2 z3",
		"frecps+fma",
	};
	printf("op=%d %s\n", op, opTbl[op]);
	for (int mode = 0; mode < sizeof(modeTbl)/sizeof(modeTbl[0]); mode++) {
		printf("mode=%s\n", modeTbl[mode]);
		Code c(op, mode);
		auto fA = c.getCode<void (*)(float *, const float *, const float *, size_t)>();
		c.ready();
		const size_t n = 1024;
		float x[n], y[n], z1[n], z2[n];
		for (size_t i = 0; i < n; i++) {
			x[i] = i + 1;
			y[i] = i;
			z1[i] = -99;
			z2[i] = z1[i];
		}
		fC(op, z1, x, y, n);
		fA(z2, x, y, n);
		bool ok = true;
		float maxe = 0;
		for (size_t i = 0; i < 16; i++) {
			float d = fabs(z1[i] - z2[i]);
			if (d > maxe) maxe = d;
			if (d > 1e-5) {
				ok = false;
				printf("i=%zd z1=%f z2=%f\n", i, z1[i], z2[i]);
			}
		}
		printf("%s maxe=%e\n", ok ? "ok" : "ng", maxe);
		CYBOZU_BENCH_C("", 1000, fA, z2, x, y, n);
		const double GHz = 2.0;
		printf("%.2fclk\n", cybozu::bench::g_clk.getClock() / double(n / 16) * GHz * 1e-3);
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
