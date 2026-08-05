/*
	Which immediates does the Golden Cove renamer fold in dependent
	add-with-immediate chains? Sweep imm sizes, then check mixed-value
	patterns (varying imm, add/sub mix, inc) to see whether identical
	immediates are required. Same calibration scheme as latency.cpp
	(8-cycle add rax, r10 chain).

	Each test runs a chain of 8 dependent instructions per iteration.
	If fully folded at rename: ~1.5 core cycles/iter. If executed: ~8.

	Pin to one core (default mask 1 = P-core on ADL; pass a mask arg to
	override, e.g. add-imm-renamer 0x20 for a Gracemont E-core) and
	re-measure the calibration around every test (min of 5 runs) to
	cancel clock drift.

	build: cl /O2 /EHsc /I c:\prog\xbyak add-imm-renamer.cpp
	   or: g++ -O2 -I ../xbyak add-imm-renamer.cpp -o add-imm-renamer
*/
/*
Core i7-1255U P-core (Golden Cove)/Windows, repeated runs agree
calib8(=8.0)  core cycles/iter=7.60
imm=1         core cycles/iter=1.52
imm=2         core cycles/iter=1.48
imm=127       core cycles/iter=2.49
imm=128       core cycles/iter=2.62
imm=255       core cycles/iter=2.89
imm=256       core cycles/iter=3.42
imm=511       core cycles/iter=3.72
imm=512       core cycles/iter=3.90
imm=1023      core cycles/iter=4.15
imm=1024      core cycles/iter=7.44
imm=1025      core cycles/iter=7.77
imm=2047      core cycles/iter=7.59
imm=2048      core cycles/iter=7.50
imm=4095      core cycles/iter=7.61
imm=4096      core cycles/iter=7.59
imm=32767     core cycles/iter=7.78
imm=32768     core cycles/iter=7.70
imm=1<<20     core cycles/iter=7.88
imm=max32     core cycles/iter=7.89
imm=-1        core cycles/iter=1.49
imm=-128      core cycles/iter=2.55
imm=-129      core cycles/iter=2.66
imm32enc 1    core cycles/iter=1.54
add 1..8      core cycles/iter=1.62
add 100+37i   core cycles/iter=3.02
add 3 / sub 2 core cycles/iter=1.61
inc x8        core cycles/iter=1.62
add/sub 1000  core cycles/iter=1.58

// w9-3495X
calib8(=8.0)  core cycles/iter=7.90
imm=1         core cycles/iter=1.91
imm=2         core cycles/iter=1.95
imm=127       core cycles/iter=2.59
imm=128       core cycles/iter=2.71
imm=255       core cycles/iter=2.95
imm=256       core cycles/iter=3.38
imm=511       core cycles/iter=4.05
imm=512       core cycles/iter=4.15
imm=1023      core cycles/iter=3.98
imm=1024      core cycles/iter=7.85
imm=1025      core cycles/iter=7.66
imm=2047      core cycles/iter=8.08
imm=2048      core cycles/iter=7.75
imm=4095      core cycles/iter=8.01
imm=4096      core cycles/iter=7.59
imm=32767     core cycles/iter=7.99
imm=32768     core cycles/iter=7.81
imm=1<<20     core cycles/iter=7.83
imm=max32     core cycles/iter=7.88
imm=-1        core cycles/iter=1.91
imm=-128      core cycles/iter=2.56
imm=-129      core cycles/iter=2.63
imm32enc 1    core cycles/iter=2.06
add 1..8      core cycles/iter=1.97
add 100+37i   core cycles/iter=3.03
add 3 / sub 2 core cycles/iter=1.94
inc x8        core cycles/iter=1.92
add/sub 1000  core cycles/iter=1.93


Core i7-1255U E-core (Gracemont): every pattern is ~7.2-8.0, no folding
at all, matching addi x8 = 8.0 in latency.cpp.

summary (Golden Cove):
- hard boundary between 1023 and 1024: an immediate outside ~[-1024, 1023]
  is never folded (straight 8.0 cycles/iter). Consistent with the renamer
  tracking "physical register + offset" with a small signed offset field
- identical immediates are NOT required: varying values (1..8), add/sub
  mixes, and inc fold just as well (~1.5). The renamer only tracks the
  running sum of offsets against the last materialized base register
- within the foldable range, larger immediates are gradually slower
  (1 -> 1.5, 127 -> 2.5, 255 -> 3, 1023 -> 4 cycles/iter): when the
  accumulated offset would overflow its field, a real add executes to
  materialize a new base register. Alternating add/sub 1000 keeps the
  accumulated sum near zero and folds fully (1.44) even though a chain
  of +1000 alone degrades to ~4, confirming the accumulated-sum model.
  imm=1 is rename-bandwidth bound (~1.5 for 8 adds)
- folding is decided by the immediate VALUE, not the encoding: value 1
  forced into the 4-byte 81 /0 id form still folds (~1.5)
- the feature is not in Intel's SDM or Optimization Reference Manual;
  it was found and documented by third-party microbenchmarks (see
  memo.md for sources). Reported limits there: imm in [-1024, 1023],
  accumulated sum within 13 bits, matching the boundary measured here
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#ifdef _WIN32
	#include <windows.h>
#endif

static const uint64_t N = 100000000;
static const int calibUnroll = 8;

enum Kind {
	UNIFORM,   // add rax, imm
	IMM32ENC,  // add rax, imm forced into the 81 /0 id encoding
	VARY1TO8,  // add rax, i+1 (i = position in the chain)
	VARY37,    // add rax, 100 + i*37
	ADDSUB,    // add rax, 3 / sub rax, 2 alternating
	INC,       // inc rax
	ADDSUB1000 // add rax, 1000 / sub rax, 1000 alternating
};

struct Test {
	const char *name;
	Kind kind;
	int64_t imm;
};

static const Test tests[] = {
	{ "imm=1        ", UNIFORM, 1 },
	{ "imm=2        ", UNIFORM, 2 },
	{ "imm=127      ", UNIFORM, 127 },
	{ "imm=128      ", UNIFORM, 128 },
	{ "imm=255      ", UNIFORM, 255 },
	{ "imm=256      ", UNIFORM, 256 },
	{ "imm=511      ", UNIFORM, 511 },
	{ "imm=512      ", UNIFORM, 512 },
	{ "imm=1023     ", UNIFORM, 1023 },
	{ "imm=1024     ", UNIFORM, 1024 },
	{ "imm=1025     ", UNIFORM, 1025 },
	{ "imm=2047     ", UNIFORM, 2047 },
	{ "imm=2048     ", UNIFORM, 2048 },
	{ "imm=4095     ", UNIFORM, 4095 },
	{ "imm=4096     ", UNIFORM, 4096 },
	{ "imm=32767    ", UNIFORM, 32767 },
	{ "imm=32768    ", UNIFORM, 32768 },
	{ "imm=1<<20    ", UNIFORM, 1 << 20 },
	{ "imm=max32    ", UNIFORM, 0x7fffffff },
	{ "imm=-1       ", UNIFORM, -1 },
	{ "imm=-128     ", UNIFORM, -128 },
	{ "imm=-129     ", UNIFORM, -129 },
	{ "imm32enc 1   ", IMM32ENC, 1 }, // value 1 but 4-byte immediate encoding
	{ "add 1..8     ", VARY1TO8, 0 },
	{ "add 100+37i  ", VARY37, 0 },
	{ "add 3 / sub 2", ADDSUB, 0 },
	{ "inc x8       ", INC, 0 },
	{ "add/sub 1000 ", ADDSUB1000, 0 },
};
static const int numTests = sizeof(tests) / sizeof(tests[0]);

struct Code : Xbyak::CodeGenerator {
	typedef void (*F)();
	F calib;
	F func[numTests];
	Code() : Xbyak::CodeGenerator(4096 * 8)
	{
		calib = gen(-1);
		for (int i = 0; i < numTests; i++) func[i] = gen(i);
	}
	F gen(int idx)
	{
		align(16);
		F f = getCurr<F>();
		mov(rax, 0);
		mov(rdx, N);
		mov(r10, 1);
		Xbyak::Label lp;
	L(lp);
		for (int i = 0; i < calibUnroll; i++) {
			if (idx < 0) {
				add(rax, r10);
				continue;
			}
			const Test& t = tests[idx];
			switch (t.kind) {
			case UNIFORM: add(rax, (uint32_t)t.imm); break;
			case IMM32ENC:
				// REX.W + 81 /0 id : add rax, imm32
				db(0x48); db(0x81); db(0xc0); dd((uint32_t)t.imm);
				break;
			case VARY1TO8: add(rax, i + 1); break;
			case VARY37: add(rax, 100 + i * 37); break;
			case ADDSUB: if (i & 1) sub(rax, 2); else add(rax, 3); break;
			case INC: inc(rax); break;
			case ADDSUB1000: if (i & 1) sub(rax, 1000); else add(rax, 1000); break;
			}
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

int main(int argc, char *argv[])
{
#ifdef _WIN32
	DWORD_PTR mask = argc > 1 ? (DWORD_PTR)strtoull(argv[1], 0, 0) : 1;
	SetThreadAffinityMask(GetCurrentThread(), mask);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
	// warm up to reach a steady clock
	for (int i = 0; i < 3; i++) measure(s_code.calib);
	// take the best (min) ratio of 5 runs, re-calibrating around each test
	double best[numTests + 1];
	for (int i = 0; i <= numTests; i++) best[i] = 1e9;
	for (int r = 0; r < 5; r++) {
		for (int i = 0; i <= numTests; i++) {
			double c = measure(s_code.calib) / calibUnroll;
			double t = measure(i == 0 ? s_code.calib : s_code.func[i - 1]);
			double c2 = measure(s_code.calib) / calibUnroll;
			double ratio = t / ((c + c2) / 2);
			if (ratio < best[i]) best[i] = ratio;
		}
	}
	printf("calib8(=8.0)  core cycles/iter=%.2f\n", best[0]);
	for (int i = 0; i < numTests; i++) {
		printf("%s core cycles/iter=%.2f\n", tests[i].name, best[i + 1]);
	}
}
