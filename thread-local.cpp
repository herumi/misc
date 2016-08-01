#include <thread>
#include <stdio.h>
#define XBYAK_NO_OP_NAMES
#include "xbyak/xbyak.h"
#include <cybozu/benchmark.hpp>

thread_local int a = 5;

int b = 3;

struct Code1 : public Xbyak::CodeGenerator {
	Code1()
	{
		const void *addr = (void*)0xffffffe0;
		putSeg(fs); inc(dword [addr]);
		putSeg(fs); mov(rax, ptr [addr]);
		ret();
	}
};

struct Code2 : public Xbyak::CodeGenerator {
	Code2()
	{
		mov(rax, (size_t)&b);
		inc(dword [rax]);
		mov(eax, ptr [rax]);
		ret();
	}
};

void ff()
{
	printf("ff %d\n", a);
}

int main()
{
	std::thread t(ff);
	printf("%d\n", a);
	Code1 c1;
	Code2 c2;
	int (*f)() = c1.getCode<int (*)()>();
	int (*g)() = c2.getCode<int (*)()>();
	printf("f=%d\n", f());
	printf("g=%d\n", g());
	CYBOZU_BENCH_C("f", 100000, f);
	CYBOZU_BENCH_C("g", 100000, g);
	t.join();
	printf("a=%d, b=%d\n", a, b);
}
