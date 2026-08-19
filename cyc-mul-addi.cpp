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
static const size_t tblN = sizeof(g_tbl) / sizeof(g_tbl[0]);

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		for (size_t mode = 0; mode < tblN; mode++) {
			g_tbl[mode].f = gen(mode);
		}
	}
	Func gen(size_t mode)
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
			for (int i = 0; i < 8; i++) add(rax, i + 1);
			break;
		case 4:
			for (int i = 0; i < 16; i++) add(rax, i + 1);
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

int main()
{
	g_rate = getCycleRate();
	fprintf(stderr, "rate=%f\n", g_rate);
	for (size_t i = 0; i < tblN; i++) {
		printf("%s %.2f\n", g_tbl[i].name, measure(g_tbl[i].f) / g_rate);
	}
}
