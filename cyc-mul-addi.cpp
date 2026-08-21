/*
perf stat -e '{instructions,uops_issued.any,uops_executed.thread,uops_retired.slots,inst_retired.macro_fused}:u'
g++ -O2 -I ../xbyak cyc-mul-addi.cpp
misc% perf stat -e '{instructions,uops_issued.any,uops_executed.thread,uops_retired.slots,inst_retired.macro_fused}:u' ./a.out 0
WARNING: events were regrouped to match PMUs
mode=0
warming up
calc rate
rate=0.411231
0 base 7.94

 Performance counter stats for './a.out 0':

       70001837791      instructions
       63002218069      uops_retired.slots
       63004854655      uops_issued.any
       63002942882      uops_executed.thread
        7000247851      inst_retired.macro_fused

      12.133653256 seconds time elapsed

      12.133252000 seconds user
       0.000000000 seconds sys


misc% perf stat -e '{instructions,uops_issued.any,uops_executed.thread,uops_retired.slots,inst_retired.macro_fused}:u' ./a.out 4
WARNING: events were regrouped to match PMUs
mode=4
warming up
calc rate
rate=0.411474
4 addix16 3.02

 Performance counter stats for './a.out 4':

       78001837738      instructions
       71030991032      uops_retired.slots
       71033292546      uops_issued.any
       59074736773      uops_executed.thread
        7000247895      inst_retired.macro_fused

      11.087078074 seconds time elapsed

      11.086695000 seconds user
       0.000000000 seconds sys
*/
#include <xbyak/xbyak_util.h>

using namespace Xbyak;
using namespace Xbyak::util;

static const uint64_t N = 1000000000;
static const int UNROLL = 8;
static double g_rate;

typedef void (*Func)();

struct Table {
	const char *name;
	Func f;
} g_tbl[] = {
	{ "base", 0 },
	{ "imul", 0 },
	{ "mulx", 0 },
	{ "addix8", 0 },
	{ "addix16", 0 },
};
static const int tblN = int(sizeof(g_tbl) / sizeof(g_tbl[0]));

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		for (int mode = 0; mode < tblN; mode++) {
			g_tbl[mode].f = gen(mode);
		}
	}
	Func gen(int mode)
	{
		Func func = getCurr<Func>();
		align(16);
		Label lpL;
		mov(ecx, N);
	L(lpL);
		switch (mode) {
		case 0:
			for (int i = 0; i < UNROLL; i++) add(rax, rax);
			break;
		case 1:
			imul(rax, rax);
			break;
		case 2:
			mulx(rax, rax, rax);
			break;
		case 3:
			for (int i = 0; i < 8; i++) add(rax, 1);
			break;
		case 4:
			for (int i = 0; i < 16; i++) add(rax, 1);
			break;
		}
		dec(ecx);
		jnz(lpL);
		ret();
		return func;
	}
} s_code;

double measure(Func f)
{
	Clock clk;
	clk.begin();
	f();
	clk.end();
	return clk.getClock() / double(N);
}

double getCycleRate()
{
	Func f = g_tbl[0].f;
	const int n = 5;
	fprintf(stderr, "warming up\n");
	measure(f);
	fprintf(stderr, "calc rate\n");
	double sum = 0;
	for (int i = 0; i < n; i++) {
		sum += measure(f) / UNROLL;
	}
	return sum / n;
}

int main(int argc, char *argv[])
{
	int mode = argc == 1 ? -1 : atoi(argv[1]);
	fprintf(stderr, "mode=%d\n", mode);
	g_rate = getCycleRate();
	fprintf(stderr, "rate=%f\n", g_rate);
	for (int i = 0; i < tblN; i++) {
		if (mode >= 0 && mode != i) continue;
		printf("%d %s %.2f\n", i, g_tbl[i].name, measure(g_tbl[i].f) / g_rate);
	}
}
