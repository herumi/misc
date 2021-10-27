#include <xbyak/xbyak_util.h>
#include <cybozu/benchmark.hpp>

using namespace Xbyak;
using namespace Xbyak::util;

typedef int (*funcType)(int n);

const char *g_callStr;

struct Code : Xbyak::CodeGenerator {
	Code(int mode)
	{
		Xbyak::util::StackFrame sf(this, 1, UseRDX);
		const Reg64& n = sf.p[0];

		Label funcL, memL;
		xor_(rax, rax);
		switch (mode) {
		case 0: mov(rdx, funcL); break;
		case 1: mov(rdx, memL); break;
		default: break;
		}
	Label lpL = L();
		switch (mode) {
		case 0: call(rdx); g_callStr = "call(rdx)"; break;
		case 1: call(ptr[rdx]); g_callStr = "call(ptr[rdx])"; break;
		case 2: call(funcL); g_callStr = "call(funcL)"; break;
		case 3: call(ptr[rip + memL]); g_callStr = "call(ptr[rip + memL])"; break;
		default : throw std::runtime_error("bad mode");
		}
		sub(n, 1);
		jnz(lpL);
		ret();

	L(funcL);
		add(rax, 1);
		ret();
	L(memL);
		dq(uint64_t(funcL.getAddress()));
	}
};

int main()
{
	const int n = 1000;
	for (int i = 0; i < 4; i++) {
		Code c(i);
		funcType f = c.getCode<funcType>();
		printf("f=%25s(%d) ", g_callStr, f(n));
		CYBOZU_BENCH_C("call", 10000, f, n);
	}
}
