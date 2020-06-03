#include <xbyak_aarch64/xbyak_aarch64.h>
#include <stdlib.h>
#include <stdio.h>

static float *dataG;

void dumpF()
{
	puts("---");
	for (int i = 0; i < 16; i++) {
		printf("%d %f\n", i, dataG[i]);
	}
	puts("---");
	exit(1);
}

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		Xbyak::Label funcL, dataL;
		ptrue(p0.s);
		ld1w(z0.s, p0, ptr(x0));
		adr(x0, dataL);
		st1w(z0.s, p0, ptr(x0));
		adr(x0, funcL);
		ldr(x0, ptr(x0));
		br(x0);
	L(funcL);
		int64_t addr = (int64_t)dumpF;
		dw(uint32_t(addr));
		dw(uint32_t(addr >> 32));
	L(dataL);
		dataG = (float*)getCurr();
		printf("dumpF=%p\n", dumpF);
		printf("dataG=%p\n", dataG);
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
	f(a);
}
