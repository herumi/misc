/*
	gcc -Ofast -march=native -mbmi
	i7-4770 CPU @ 3.40GHz with gcc-4.8.1
	clk 7.10 ; C
	clk 6.81 ; intrinsic
	clk 6.25 ; asm (use shrx) => 6.31 use shr(x, c), but shr(x, 1) is faster than shrx
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <cybozu/bit_operation.hpp>

#define USE_INTRIN

#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

#ifdef _MSC_VER
	#pragma warning(disable : 4146)
	#include <intrin.h>
#else
	#include <x86intrin.h>
#endif

bool g_isHaswell = false;

struct NextCombinationCode : Xbyak::CodeGenerator {
	NextCombinationCode()
		try
	{
		Xbyak::util::Cpu cpu;
		if (!cpu.has(Xbyak::util::Cpu::tBMI1)) {
			printf("This CPU does not support blsi, ...\n");
			exit(1);
		}
		g_isHaswell = true;
#ifdef XBYAK32
		const Xbyak::Reg32& a = eax;
		const Xbyak::Reg32& c = ecx;
		const Xbyak::Reg32& x = edx;
		mov(x, ptr [esp + 4]);
#else
		const Xbyak::Reg64& a = rax;
		const Xbyak::Reg64& c = rcx;
#ifdef XBYAK64_WIN
		const Xbyak::Reg64& x = r8;
		mov(r8, rcx);
#else
		const Xbyak::Reg64& x = rdi;
#endif
#endif
		test(x, 1);
		jz("@f");
		lea(a, ptr [x + 1]);
		blsmsk(a, a); // y = (x+1) ^ (x+1-1) = x ^ (x+1)
		lea(c, ptr [a + 1]); // y + 1
		shr(a, 1); // y / 2
		bsr(c, c); // rcx = bsr(y + 1)
//tzcnt(rcx, rcx);
		sub(x, a); // c
		mov(a, x);
		blsi(x, x); // c & (-c)
		shrx(x, x, rcx);
		sub(a, x);
		ret();
	L("@@");
		mov(a, x);
		blsi(x, x); // x & -x
		shr(x, 1);
		sub(a, x);
		ret();
	} catch (std::exception& e) {
		printf("ERR %s\n", e.what());
	}
} s_code;

/*
	a = 0x7fffffff => b + 1 = 0, c = 0, then return 0 if bsr(b + 1) is any
	a = 0xffffffff => b + 1 = 0, c = 0x80000000
*/
size_t nextCombinationC(size_t  a)
{
	if (a & 1) {
		size_t b = a ^ (a + 1);
		size_t c = a - b / 2;
		assert(b + 1);
#if 1
		return c - ((c & -c) >> cybozu::bsr(b + 1));
#else
		return c - (c & -c) / (b + 1);
#endif
	} else {
		return a - (a & -a) / 2;
	}
}

#ifdef USE_INTRIN
#ifdef __GNUC__
uint64_t blsmsk(uint64_t x) { return __blsmsk_u64(x); }
uint64_t blsi(uint64_t x) { return __blsi_u64(x); }
#else
uint64_t blsmsk(uint64_t x) { return _blsmsk_u64(x); }
uint64_t blsi(uint64_t x) { return _blsi_u64(x); }
#endif
size_t nextCombinationI(size_t  a)
{
	if (a & 1) {
		size_t b = blsmsk(a + 1);
		size_t c = a - b / 2;
		return c - (blsi(c) >> cybozu::bsr(b + 1));
	} else {
		return a - blsi(a) / 2;
	}
}
#endif

size_t (*nextCombination)(size_t) = g_isHaswell ? s_code.getCode<size_t (*)(size_t)>() : nextCombinationC;

void putB(size_t n, size_t a)
{
	for (size_t i = 0; i < n; i++) {
		printf("%c ", 1 & (a >> (n - 1 - i)) ? '1' : '0');
	}
	printf("\n");
}

/*
	11...100....00
	      <- k  ->
	<---  n  ---->
*/
size_t makeInit(uint32_t n, uint32_t k)
{
	return ((size_t(1) << (n - k)) - 1) << k;
}

void test()
{
	if (!g_isHaswell) return;
	uint32_t n = 13;
	uint32_t k = 7;
	size_t a = makeInit(n, k);
	while (a) {
		size_t b = nextCombination(a);
		a = nextCombinationC(a);
		if (a != b) {
			puts("ERR");
			putB(n, a);
			putB(n, b);
			exit(1);
		}
	}
	puts("test ok");
}

size_t testLoop(size_t (*f)(size_t), size_t a)
{
	size_t n = 0;
	while (a) {
		a = f(a);
		n++;
	}
	return n;
}

void bench()
{
	typedef size_t (*func)(size_t);
	const func tbl[] = {
		nextCombinationC,
#ifdef USE_INTRIN
		nextCombinationI,
#endif
		nextCombination
	};
	size_t a = makeInit(31, 15);
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		Xbyak::util::Clock clk;
		clk.begin();
		size_t n = testLoop(tbl[i], a);
		clk.end();
		printf("clk %.2f\n", clk.getClock() / double(n));
		if (!g_isHaswell) break;
	}
}

int main(int argc, char *argv[])
{
#ifdef USE_INTRIN
	printf("use intrin version\n");
#else
	printf("use %s version\n", g_isHaswell ? "asm" : "C");
#endif
	argc--, argv++;
	if (argc != 2) {
		printf("nCk n k\n");
		test();
		bench();
		return 1;
	}
	const uint32_t n = atoi(argv[0]);
	const uint32_t k = atoi(argv[1]);
	if (n >= 64 || k > n) {
		printf("bad n=%u, k=%u\n", n, k);
		return 1;
	}
	size_t a = makeInit(n, k);
	if (a == 0) {
		putB(n, a);
		return 0;
	}
	while (a) {
		putB(n, a);
		a = nextCombination(a);
	}
}
