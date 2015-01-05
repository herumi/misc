/*
	compare between constant multiplication and bit shift
*/
#define XBYAK_NO_OP_NAMES
#include <stdio.h>
#include <xbyak/xbyak_util.h>

const int N = 1000000;

struct Code0 : public Xbyak::CodeGenerator {
	Code0()
	{
		push(ebx);
		xor_(eax, eax);
		mov(edx, N);
	L("@@");
		lea(ebx, ptr [edx * 4]);
		lea(ebx, ptr [ebx + edx * 2]);
		lea(ebx, ptr [ebx + edx * 8]);
		add(eax, ebx);
		mov(ebx, edx);
		shl(ebx, 4);
		add(eax, ebx);
		dec(edx);
		jnz("@b");
		pop(ebx);
		ret();
	}
};

struct Code1 : public Xbyak::CodeGenerator {
	Code1()
	{
		xor_(eax, eax);
		mov(edx, N);
	L("@@");
		mov(ecx, edx);
		imul(ecx, ecx, 30);
		add(eax, ecx);
		dec(edx);
		jnz("@b");
		ret();
	}
};

struct Code2 : public Xbyak::CodeGenerator {
	Code2()
	{
		push(ebx);
		xor_(eax, eax);
		mov(edx, N);
	L("@@");
		mov(ebx, edx);
		shl(ebx, 4);
		sub(ebx, edx);
		add(ebx, ebx);
		add(eax, ebx);
		dec(edx);
		jnz("@b");
		pop(ebx);
		ret();
	}
};

struct Code3 : public Xbyak::CodeGenerator {
	Code3()
	{
		push(ebx);
		xor_(eax, eax);
		mov(edx, N);
	L("@@");
		mov(ebx, edx);
		shl(ebx, 4);
		sub(ebx, edx);
		lea(eax, ptr [eax + ebx * 2]);
		dec(edx);
		jnz("@b");
		pop(ebx);
		ret();
	}
};

template<class C>
void test()
{
	const int M = 100;
	int x = 0;
	C code;
	int (*f)() = (int (*)())code.getCode();
	Xbyak::util::Clock clk;
	for (int i = 0; i < M; i++) {
		clk.begin();
		x += f();
		clk.end();
	}
	printf("x=%d, %.2fclk\n", x, clk.getClock() / double(M) / N);
}

int main()
{
	test<Code0>();
	test<Code1>();
	test<Code2>();
	test<Code3>();
}
