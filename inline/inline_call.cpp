#include <xbyak/xbyak.h>
#include <cybozu/benchmark.hpp>

using namespace Xbyak;
using namespace Xbyak::util;

void (*f1)();
void (*f2)();

const int N = 1000;

struct Code : CodeGenerator {
	Code()
	{
		gen1();
		gen2();
	}
	void gen1()
	{
		f1 = getCurr<void (*)()>();
		mov(ecx, N);
		xor_(eax, eax);
		Label lp;
	L(lp);
		add(eax, 3);
		imul(eax, eax);
		dec(ecx);
		jnz(lp);
		ret();
	}
	void gen2()
	{
		f2 = getCurr<void (*)()>();
		mov(ecx, N);
		xor_(eax, eax);
		Label lp, funcL;
	L(lp);
		call(funcL);
		dec(ecx);
		jnz(lp);
		ret();
		align(32);
	L(funcL);
		add(eax, 3);
		imul(eax, eax);
		ret();
	}
};

int main()
	try
{
	Code c;
	CYBOZU_BENCH_C("f1", 1000, f1);
	CYBOZU_BENCH_C("f2", 1000, f2);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}

