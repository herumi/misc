#include <stdio.h>
#include <xbyak/xbyak.h>
#include <stdint.h>

typedef uint64_t (*mul_t)(uint64_t x, uint64_t y);

mul_t f1;
mul_t f2;

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		gen1();
		gen2();
	}
	// Win64 : rcx, rdx
	// Linux : rdi, rsi
	void gen1()
	{
		f1 = getCurr<mul_t>();
#ifdef XBYAK_WIN64
		mov(rax, rdx);
		mul(rcx);
#else
		mov(rax, rsi);
		mul(rdi);
#endif
		ret();
	}
	void gen2()
	{
		f2 = getCurr<mul_t>();
#ifdef XBYAK_WIN64
		mov(rax, rcx);
		imul(rax, rdx);
#else
		mov(rax, rdi);
		imul(rax, rsi);
#endif
		ret();
	}
} g_code;

int main()
	try
{
	const struct Pair {
		uint64_t x;
		uint64_t y;
	} tbl[] = {
		{ 2, 3 },
		{ uint64_t(-2), 3 },
		{ 2, uint64_t(-3) },
		{ uint64_t(-2), uint64_t(-3) },
		{ 0x123456789ull, 0x123456789aull },
		{ 0xff00000011223344ull, 0x12345678 },
	};
	for (size_t i = 0; i < sizeof(tbl) / sizeof(tbl[0]); i++) {
		uint64_t x = tbl[i].x;
		uint64_t y = tbl[i].y;
		uint64_t a = f1(x, y);
		uint64_t b = f2(x, y);
		int64_t c = int64_t(x) * int64_t(y);
		uint64_t d = x * y;
		if (a != b) {
			printf("ERR1 x=%llx, y=%llx, a=%llx b=%llx c=%llx\n", (long long)x, (long long)y, (long long)a, (long long)b, (long long)c);
		}
		if (uint64_t(c) != d) {
			printf("ERR2 x=%llx, y=%llx, c=%llx d=%llx\n", (long long)x, (long long)y, (long long)c, (long long)d);
		}
	}
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
