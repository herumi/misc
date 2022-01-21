#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak_aarch64;

void putVal(int x)
{
	printf("x=%d\n", x);
}

struct Code : CodeGenerator {
	Code(bool stop)
	{
		Label tblL, incL, addL;
		if (stop) brk(0);
		mov(x0, 0);
		// exec
		ret();
	L(tblL);
		putL(incL);
		putL(addL);

	L(incL);
		add(x0, x0, 1);
		ret();
	L(addL);
		add(x0, x0, 5);
		ret();
	}
};

int main(int argc, char *[])
{
	Code c(argc > 1);
	auto f = c.getCode<int (*)()>();
	int v = f();
	printf("v=%d\n", v);
	puts("end");
}

