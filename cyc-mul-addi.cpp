/*
perf stat -e '{instructions,uops_issued.any,uops_executed.thread,uops_retired.slots,inst_retired.macro_fused}:u'
g++ -O2 -I ../xbyak cyc-mul-addi.cpp
*/
#include <xbyak/xbyak_util.h>
#include <string_view>

using namespace Xbyak;
using namespace Xbyak::util;

static const uint64_t N = 1000000000;
static const int UNROLL = 8;
static double g_rate = 1;

typedef void (*Func)();

struct Table {
	const char *name;
	Func f;
} g_tbl[] = {
	{ "base", 0 }, // 0
	{ "imul", 0 }, // 1
	{ "mulx", 0 }, // 2
	{ "addix8", 0 }, // 3
	{ "addix16", 0 }, // 4
	{ "add32ix8", 0 }, // 5
	{ "addrix8", 0 }, // 6
	{ "addix5", 0 }, // 7
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
		const int regN = 8;
		Func func = getCurr<Func>();
		StackFrame sf(this, 0, regN|UseRCX);
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
		case 5:
			for (int i = 0; i < regN; i++) add(eax, 1);
			break;
		case 6:
			for (int i = 0; i < regN; i++) add(sf.t[i], 1);
			break;
		case 7:
			for (int i = 0; i < 5; i++) add(rax, 1);
			break;
		default:
			fprintf(stderr, "ERR bad mode=%d\n", mode);
			exit(1);
		}
		dec(ecx);
		jnz(lpL);
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
	const std::string_view nowarm = "-nowarm";
	bool warm = true;
	const char *unit = "tsc";
	if (argc > 1 && nowarm == argv[1]) {
		warm = false;
		argc--, argv++;
	}
	int mode = argc == 1 ? -1 : atoi(argv[1]);
	fprintf(stderr, "mode=%d\n", mode);
	if (warm) {
		g_rate = getCycleRate();
		unit = "cyc";
		fprintf(stderr, "rate=%f\n", g_rate);
	} else {
		fprintf(stderr, "nowarm\n");
	}
	for (int i = 0; i < tblN; i++) {
		if (mode >= 0 && mode != i) continue;
		printf("%d %s %.2f %s\n", i, g_tbl[i].name, measure(g_tbl[i].f) / g_rate, unit);
	}
}
