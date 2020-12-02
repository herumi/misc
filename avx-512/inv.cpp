#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#include <cybozu/benchmark.hpp>
#include <math.h>
/*
	clang -O3
float x[1024]
3.208usec ; 100  clk ; fdiv
1.058usec ;  33  clk ; frintm and frecps use same z2
0.350usec ;  10.9clk ; frintm and frecps use diff regs
1.651usec ;  51  clk ; frintm and frecps x 2 use same z2
0.370usec ;  11.2clk ; frintm and frecps x 2 use diff reg
*/

using namespace Xbyak;

struct Code : CodeGenerator {
	Code(int op, int mode)
	{
		static float f1 = 1.0f;
		util::StackFrame sf(this, 4);
		const auto& out = sf.p[0];
		const auto& src1 = sf.p[1];
		const auto& src2 = sf.p[2];
		const auto& n = sf.p[3];
		const auto& one = zm4;
		const auto& two = zm5;
		mov(rax, (size_t)&f1);
		vbroadcastss(one, ptr[rax]);
		vaddps(two, one, one);
		// assume n > 0 && (n % 16) == 0
	Label lp = L();
		vmovups(zm0, ptr[src1]);
		vmovups(zm1, ptr[src2]);
		add(src1, 64);
		add(src2, 64);
		switch (op) {
		case 0:
			vrndscaleps(zm2, zm0, 2); // floor
			break;
		case 1:
			vaddps(zm2, zm0, zm0);
			break;
		}
		vaddps(zm0, zm1, zm2);
		switch (mode) {
		case 0:
			vdivps(zm0, one, zm0);
			break;
		case 1:
			vrcp14ps(zm0, zm0);
			break;
		case 2:
			vrcp14ps(zm2, zm0);
			vaddps(zm1, zm2, zm2);
			vmulps(zm2, zm2, zm2);
			vmulps(zm2, zm2, zm0);
			vsubps(zm0, zm1, zm2);
			break;
		case 3:
			/*
				t = rcp14(x)
				1/x = (-xt + 2)t
			*/
			vrcp14ps(zm1, zm0); // t
			vfnmadd213ps(zm0, zm1, two);
			vmulps(zm0, zm0, zm1);
			break;
		}
		vmovups(ptr[out], zm0);
		add(out, 64);
		sub(n, 16);
		jnz(lp);
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
			v = 1;
			break;
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
		"vrcp14ps",
		"vrcp14ps+approx",
		"vrcp14ps+vfnmadd",
	};
	printf("op=%d %s\n", op, opTbl[op]);
	for (int mode = 0; mode < 4; mode++) {
		printf("mode=%s\n", modeTbl[mode]);
		Code c(op, mode);
		auto fA = c.getCode<void (*)(float *, const float *, const float *, size_t)>();
		c.ready();
		const size_t n = 1024 * 4;
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
		CYBOZU_BENCH_C("", 10000, fA, z2, x, y, n);
		printf("%.2fclk\n", cybozu::bench::g_clk.getClock() / double(n / 16) * 2 * 1e-3);
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
