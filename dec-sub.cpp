#include <stdio.h>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>

const size_t N = 100000000;

struct Code : Xbyak::CodeGenerator {
	explicit Code(bool useDec)
	{
		puts(useDec ? "use dec" : "use sub");
		mov(eax, N);
	L("@@");
		if (useDec) {
			sub(eax, 1);
		} else {
			dec(eax);
		}
		jnz("@b");
		ret();
	}
};

int main(int argc, char *argv[])
{
	int mode = argc == 1 ? -1 : atoi(argv[1]);
	for (int i = 0; i < 2; i++) {
		if (mode >= 0 && mode != i) continue;
		Code c(i == 0);
		void (*f)() = c.getCode<void (*)()>();
		Xbyak::util::Clock clk;
		clk.begin();
		f();
		clk.end();
		printf("%.2fclk\n", clk.getClock() / double(N));
	}
}

