#include <xbyak/xbyak_util.h>

using namespace Xbyak;
using namespace Xbyak::util;

typedef void (*Func)();

static const uint64_t N = 1000000000;
static const int UNROLL = 8;

struct Code : Xbyak::CodeGenerator {
	Code()
		: f0(gen(0))
		, f1(gen(1))
		, f2(gen(2))
	{

	}
	Func f0, f1, f2;
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
		}
		dec(ecx);
		jnz(lpL);
		ret();
		return func;
	}
} s_c;

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
	Func f = s_c.f0;
	const int n = 5;
	measure(f);
	double sum = 0;
	for (int i = 0; i < n; i++) {
		sum += measure(f) / UNROLL;
	}
	return sum / n;
}

int main()
{
	const double rate = getCycleRate();
	printf("rate=%f\n", rate);
	printf("mul %.2f cyc\n", measure(s_c.f1) / rate);
	printf("addi %.2f cyc\n", measure(s_c.f2) / rate);
}
