#include <xbyak/xbyak.h>
#include <cybozu/benchmark.hpp>

using namespace Xbyak;

typedef int (*funcType)();

const char *g_callStr;

struct Code : Xbyak::CodeGenerator {
	bool gen(int loopN, int mode)
	{
		const Reg64& n = rcx;
		const Reg64& addr = r8;

		Label funcL, memL, tblL;
		xor_(rax, rax);
		mov(n, loopN);
		switch (mode) {
		case 0: mov(rdx, funcL); break;
		case 1: mov(rdx, memL); break;
		case 4: mov(addr, tblL); break;
		default: break;
		}
	Label lpL = L();
		switch (mode) {
		case 0: call(rdx); g_callStr = "call(rdx)"; break;
		case 1: call(ptr[rdx]); g_callStr = "call(ptr[rdx])"; break;
		case 2: call(funcL); g_callStr = "call(funcL)"; break;
		case 3: call(ptr[rip + memL]); g_callStr = "call(ptr[rip + memL])"; break;
		case 4: mov(rdx, ptr[addr]); call(ptr[rdx]); g_callStr = "mov(rdx, [rdx]); call([rdx])"; break;
		default : return false;
		}
		sub(n, 1);
		jnz(lpL);
		ret();

	L(funcL);
		add(rax, 1);
		ret();
	L(memL);
		dq(uint64_t(funcL.getAddress()));
	L(tblL);
		dq(uint64_t(memL.getAddress()));
		return true;
	}
};

int main()
{
	const int n = 1000;
	for (int i = 0; i < 10; i++) {
		Code c;
		if (!c.gen(n, i)) break;
		funcType f = c.getCode<funcType>();
		printf("f=%25s(%d) ", g_callStr, f());
		CYBOZU_BENCH_C("", 10000, f);
		CYBOZU_BENCH_C("call", 10000, f);
	}
}
