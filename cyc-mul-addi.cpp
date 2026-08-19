#include <xbyak/xbyak_util.h>

using namespace Xbyak;
using namespace Xbyak::util;

typedef void (*Func)();

static const uint64_t N = 1000000000;
static const int UNROLL = 8;
static double g_rate;

struct Code : Xbyak::CodeGenerator {
	Code()
		: f0(gen(0))
		, f1(gen(1))
		, f2(gen(2))
		, f3(gen(3))
	{

	}
	Func f0, f1, f2, f3;
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
			for (int i = 0; i < UNROLL * 2; i++) add(rax, i + 1);
			break;
		case 3:
			mulx(rax, rax, rax);
			break;
		}
		dec(ecx);
		jnz(lpL);
		ret();
		return func;
	}
} s_c;

double measureBase(Func f)
{
	Clock clk;
	clk.begin();
	f();
	clk.end();
	return clk.getClock() / double(N);
}

double getCycleRate()
{
	Func f = s_c.f0;
	const int n = 5;
	fprintf(stderr, "warming up\n");
	measureBase(f);
	fprintf(stderr, "calc rate\n");
	double sum = 0;
	for (int i = 0; i < n; i++) {
		sum += measureBase(f) / UNROLL;
	}
	return sum / n;
}

void measure(const char *msg, Func f)
{
	printf("%s %.2f\n", msg, measureBase(f) / g_rate);
}

int main()
{
	g_rate = getCycleRate();
	fprintf(stderr, "rate=%f\n", g_rate);
	measure("mul", s_c.f1);
	measure("addi", s_c.f2);
	measure("mulx", s_c.f3);
}
