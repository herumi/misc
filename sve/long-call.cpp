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

void genDebug(Xbyak::CodeGenerator *g, const Xbyak::ZReg& z)
{
	const auto& x0 = g->x0;
	const auto& x1 = g->x1;
	const auto& x29 = g->x29;
	const auto& x30 = g->x30;
	const auto& sp = g->sp;
	const auto& p0 = g->p0;
	using namespace Xbyak;
	Label funcL, mainL, exitL;
	const int xN = 16;
	const int zN = 24;
	int stackSize = (xN + 2) * 8 + zN * 64;
	// save registers
	g->stp(x29, x30, pre_ptr(sp, -16));
	g->sub(sp, sp, stackSize);
	for (int i = 0; i < xN; i++) {
		g->str(XReg(i), ptr(sp, i * 8));
	}
	g->str(p0, ptr(sp, xN * 8));
	g->add(x0, sp, (xN + 2) * 8);
	for (int i = 0; i < zN; i++) {
		g->st1w(ZReg(i).s, p0, ptr(x0));
		g->add(x0, x0, 64);
	}
	g->b(mainL);
	g->align(16);
g->L(funcL);
	uint64_t funcAddr = (uint64_t)dumpF;
	printf("func=%016lx\n", funcAddr);
	g->dw(uint32_t(funcAddr));
	g->dw(uint32_t(funcAddr >> 32));
g->L(mainL);
	g->ptrue(p0.s);
	g->sub(sp, sp, 64);
	g->st1w(z.s, p0, ptr(sp)); // save z to sp[]
	g->mov(x0, sp);
	g->ldr(x1, funcL);
	g->blr(x1); // call funcL(sp)
	g->add(sp, sp, 64);

	// restore registers
	g->add(x0, sp, (xN + 2) * 8);
	for (int i = 0; i < zN; i++) {
		g->ld1w(ZReg(i).s, p0, ptr(x0));
		g->add(x0, x0, 64);
	}
	for (int i = 0; i < xN; i++) {
		g->ldr(XReg(i), ptr(sp, i * 8));
	}
	g->ldr(p0, ptr(sp, xN * 8));
	g->add(sp, sp, stackSize);
	g->ldp(x29, x30, post_ptr(sp, 16));
}

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		ptrue(p0.s);
		ld1w(z0.s, p0, ptr(x0));
		genDebug(this, z0);
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
