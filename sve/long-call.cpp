#include <xbyak_aarch64/xbyak_aarch64.h>
#include <stdlib.h>
#include <stdio.h>

static void dumpF(const float *x)
{
	printf("x=%p\n", x);
	puts("---");
	for (int i = 0; i < 16; i++) {
		printf("%d %f\n", i, x[i]);
	}
	puts("---");
}

struct Code : Xbyak::CodeGenerator {
	void genDebug(const Xbyak::ZReg& z)
	{
		using namespace Xbyak;
		Label funcL, skipL, exitL;
		const int xN = 16;
		const int zN = 24;
		int stackSize = (xN + 2) * 8 + zN * 64;
		// save registers
		stp(x29, x30, pre_ptr(sp, -16));
		sub(sp, sp, stackSize);
		for (int i = 0; i < xN; i++) {
			str(XReg(i), ptr(sp, i * 8));
		}
		str(p0, ptr(sp, xN * 8));
		add(x0, sp, (xN + 2) * 8);
		for (int i = 0; i < zN; i++) {
			st1w(ZReg(i).s, p0, ptr(x0));
			add(x0, x0, 64);
		}
		b(skipL);
		align(16);
	L(funcL);
		uint64_t funcAddr = (uint64_t)dumpF;
		printf("func=%016lx\n", funcAddr);
		dw(uint32_t(funcAddr));
		dw(uint32_t(funcAddr >> 32));
	L(skipL);
		sub(sp, sp, 64);
		st1w(z.s, p0, ptr(sp));
		add(x0, sp, 0);
		ldr(x1, funcL);
		blr(x1);
		add(sp, sp, 64);

		// restore registers
		add(x0, sp, (xN + 2) * 8);
		for (int i = 0; i < zN; i++) {
			ld1w(ZReg(i).s, p0, ptr(x0));
			add(x0, x0, 64);
		}
		for (int i = 0; i < xN; i++) {
			ldr(XReg(i), ptr(sp, i * 8));
		}
		ldr(p0, ptr(sp, xN * 8));
		add(sp, sp, stackSize);
		ldp(x29, x30, post_ptr(sp, 16));
	}
	Code()
	{
		ptrue(p0.s);
		ld1w(z0.s, p0, ptr(x0));
		genDebug(z0);
		ret();
	}
};

int main()
{
	Code c;
	auto f = c.getCode<void (*)(float*)>();
	puts("call");
	float a[16];
	for (int i = 0; i < 16; i++) {
		a[i] = i * 0.1f;
	}
	float b[16];
	for (int i = 0; i < 16; i++) {
		b[i] = i * i;
	}
	f(a);
	puts("ok1");
	f(b);
	puts("ok2");
}
