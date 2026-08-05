/*
	Measure L1 load-to-use latency and store-to-load forwarding latency
	in core cycles, calibrated by a known 8-cycle dependency chain
	(add rax, r10 x 8) so that no perf counter access is needed.

	core cycles/iter = tsc(test)/iter / (tsc(calib)/iter / 8)
*/
/*
Xeon Platinum 8280 (Cascade Lake)
calib8(=8.0) tsc=5.411 core cycles/iter=8.00
calib3(=3.0) tsc=2.028 core cycles/iter=3.00
load         tsc=2.704 core cycles/iter=4.00
sf           tsc=3.037 core cycles/iter=4.49
sf+add       tsc=3.692 core cycles/iter=5.46
rmw          tsc=3.662 core cycles/iter=5.41
rmw split    tsc=3.685 core cycles/iter=5.45
sf indexed   tsc=3.039 core cycles/iter=4.49
addi x8      tsc=5.408 core cycles/iter=8.00

Xeon w9-3495X (Sapphire Rapids)
calib8(=8.0) tsc=3.511 core cycles/iter=8.02
calib3(=3.0) tsc=1.293 core cycles/iter=2.95
load         tsc=2.221 core cycles/iter=5.07
sf           tsc=0.555 core cycles/iter=1.27
sf+add       tsc=0.491 core cycles/iter=1.12
rmw          tsc=3.246 core cycles/iter=7.41
rmw split    tsc=0.430 core cycles/iter=0.98
sf indexed   tsc=2.818 core cycles/iter=6.43
addi x8      tsc=0.864 core cycles/iter=1.97

Core i7-1255U 2.6GHz/Windows
calib8(=8.0) tsc=4.741 core cycles/iter=8.02
calib3(=3.0) tsc=1.794 core cycles/iter=3.03
load         tsc=2.980 core cycles/iter=5.04
sf           tsc=0.603 core cycles/iter=1.02
sf+add       tsc=0.624 core cycles/iter=1.05
rmw          tsc=4.405 core cycles/iter=7.45
rmw split    tsc=0.598 core cycles/iter=1.01
sf indexed   tsc=3.017 core cycles/iter=5.10
addi x8      tsc=0.956 core cycles/iter=1.62

Core i7-1255U E-core (Gracemont)
>start /b /wait /affinity 20 latency.exe
calib8(=8.0) tsc=6.142 core cycles/iter=8.01
imul(=3|E:5) tsc=3.808 core cycles/iter=4.97
load         tsc=2.289 core cycles/iter=2.99
sf           tsc=0.766 core cycles/iter=1.00
sf+add       tsc=0.959 core cycles/iter=1.25
rmw          tsc=0.783 core cycles/iter=1.02
rmw split    tsc=0.951 core cycles/iter=1.24
sf indexed   tsc=1.072 core cycles/iter=1.40
addi x8      tsc=6.540 core cycles/iter=8.53

summary (CLX -> SPR):
- L1 load-to-use : 4.0 -> 5.0
- store forwarding roundtrip (non-renamed, sf indexed) : 4.5 -> 6.4
- rmw add [mem],reg : 5.4 -> 7.4 (matches perf: 5.42 / 7.39)
- SPR renames explicit mov store/load pairs (sf, rmw split ~1 cycle);
  CLX does not. Indexed addressing and the RMW instruction do not benefit
- SPR folds dependent immediate-add chains (~4 adds/cycle); CLX does not
- i7-1255U (Alder Lake, Golden Cove P-core) matches SPR: load 5.0,
  rmw 7.45, renamed mov pairs ~1.0, immediate-add folding. Only the
  non-renamed forwarding (sf indexed) is lower : 5.1 vs 6.4 on SPR
- Gracemont E-core: load 3.0, and store forwarding is ~1-1.5 cycles for
  every pattern including the RMW instruction (rmw 1.0!) and indexed
  addressing (1.5). No immediate-add folding (addi x8 = 8.0). In core
  cycles this loop runs 7x faster on the E-core than on the P-core
*/
#include <stdio.h>
#include <stdint.h>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

static const uint64_t N = 1000000000;
static const int calibUnroll = 8;
alignas(64) static uint64_t buf[8];

struct Code : Xbyak::CodeGenerator {
	typedef void (*F)();
	F calib1, calib3, load, sf, sfadd, rmw, rmwSplit, sfIdx, addImm;
	Code() : Xbyak::CodeGenerator(4096 * 4)
	{
		calib1 = gen(0);
		calib3 = gen(1);
		load = gen(2);
		sf = gen(3);
		sfadd = gen(4);
		rmw = gen(5);
		rmwSplit = gen(6);
		sfIdx = gen(7);
		addImm = gen(8);
	}
	F gen(int mode)
	{
		align(16);
		F f = getCurr<F>();
		mov(rax, 0);
		mov(rcx, (size_t)buf);
		mov(rdx, N);
		mov(r9, 0);
		mov(r10, 1);
		Xbyak::Label lp;
	L(lp);
		switch (mode) {
		case 0:
			/*
				8 cycles/iter chain of register-source adds. This avoids two
				pitfalls that broke earlier calibrations:
				(a) SKX/CLX frontend limit: a single-add loop runs at
				    2 cycles/iter (LSD disabled + JCC erratum)
				(b) SPR folds dependent immediate-add chains at the renamer
				    (~4 adds/cycle), see case 8
			*/
			for (int i = 0; i < calibUnroll; i++) add(rax, r10);
			break;
		case 1: // imul latency chain : 3 cycles on big cores, 5 on Gracemont
			imul(rax, rax);
			break;
		case 2: // L1 load-to-use : pointer chase, buf[0] = &buf[0]
			mov(rcx, ptr [rcx]);
			break;
		case 3: // store-to-load forwarding roundtrip
			mov(ptr [rcx], rax);
			mov(rax, ptr [rcx]);
			break;
		case 4: // forwarding + 1 ALU op (mimics RMW chain by separate uops)
			mov(ptr [rcx], rax);
			mov(rax, ptr [rcx]);
			add(rax, 1);
			break;
		case 5: // RMW as in call-nocall.cpp (rax is not the counter here)
			add(ptr [rcx], rax);
			break;
		case 6: // split RMW (the #else branch of call-nocall.cpp)
			mov(r8, ptr [rcx]);
			add(r8, rax);
			mov(ptr [rcx], r8);
			break;
		case 7: // sf with indexed addressing (may defeat memory renaming)
			mov(ptr [rcx + r9], rax);
			mov(rax, ptr [rcx + r9]);
			break;
		case 8:
			/*
				dependent immediate-add chain. 8.0 on CLX, but ~2.0 on SPR
				because the renamer folds add-with-immediate chains
			*/
			for (int i = 0; i < calibUnroll; i++) add(rax, 1);
			break;
		}
		sub(rdx, 1);
		jnz(lp);
		ret();
		return f;
	}
} s_code;

double measure(void (*f)())
{
	Xbyak::util::Clock clk;
	clk.begin();
	f();
	clk.end();
	return clk.getClock() / double(N);
}

int main()
{
	buf[0] = (uint64_t)buf;
	// calibrate repeatedly until two consecutive runs agree within 1%
	double c1 = measure(s_code.calib1) / calibUnroll;
	for (int i = 0; i < 10; i++) {
		double t = measure(s_code.calib1) / calibUnroll;
		bool stable = t > c1 * 0.99 && t < c1 * 1.01;
		c1 = t;
		if (stable) break;
	}
	double c3 = measure(s_code.calib3) / c1;
	bool imul3 = c3 > 2.9 && c3 < 3.1; // big cores
	bool imul5 = c3 > 4.9 && c3 < 5.1; // Gracemont E-cores
	if (!imul3 && !imul5) {
		printf("warning: calibration unstable (imul=%.2f, expected 3.0 or 5.0)\n", c3);
	}
	struct { const char *name; void (*f)(); } tbl[] = {
		{ "calib8(=8.0)", s_code.calib1 },
		{ "imul(=3|E:5)", s_code.calib3 },
		{ "load        ", s_code.load },
		{ "sf          ", s_code.sf },
		{ "sf+add      ", s_code.sfadd },
		{ "rmw         ", s_code.rmw },
		{ "rmw split   ", s_code.rmwSplit },
		{ "sf indexed  ", s_code.sfIdx },
		{ "addi x8     ", s_code.addImm },
	};
	for (const auto& e : tbl) {
		double t = measure(e.f);
		printf("%s tsc=%.3f core cycles/iter=%.2f\n", e.name, t, t / c1);
	}
}
