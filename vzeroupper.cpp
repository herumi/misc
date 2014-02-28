/*
	see Intel manual Sytem Programming Guide
	Chapter 19 Performance-Monitoring Events
	perf -e r08c1 ./a.out ; OTHER_ASSISTS.AVX_TO_SSE
	perf -e r10c1 ./a.out ; OTHER_ASSISTS.SSE_TO_AVX
*/
#include <stdio.h>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

const size_t N = 10000;

struct Code : public Xbyak::CodeGenerator {
	explicit Code(int mode)
	{
		mov(eax, N);
	L("@@");
		vaddpd(ym0, ym0);

		if (mode & 1) vzeroupper();

		addpd(xm0, xm0);

		if (mode & 2) vzeroupper();

		vaddpd(ym0, ym0);
		sub(eax, 1);
		jnz("@b");
		ret();
	}
};

int main(int argc, char *argv[])
{
	int sel = argc == 1 ? -1 : atoi(argv[1]);
	for (int mode = 0; mode < 4; mode++) {
		if (sel >= 0 && sel != mode) continue;
		Code c(mode);
		void (*f)() = c.getCode<void (*)()>();
		Xbyak::util::Clock clk;
		const size_t C = 1000;
		clk.begin();
		for (size_t i = 0; i < C; i++) {
			f();
		}
		clk.end();
		printf("mode=%d %7.3fclk\n", mode, (double)clk.getClock () / C / N);
	}
}
