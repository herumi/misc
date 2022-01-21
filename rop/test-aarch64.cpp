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
		int saveByte = 16;
		stp(x29, x30, pre_ptr(sp, -saveByte));
		mov(x0, 0);
#if 1
		adr(x1, incL);
		blr(x1);
#else
		adr(x2, addL);
		blr(x2);
		adr(x2, incL);
		blr(x2);
#endif
		// exec
		ldp(x29, x30, post_ptr(sp, saveByte));
		ret();

	L(incL);
		add(x0, x0, 1);
		ret();
	L(addL);
		add(x0, x0, 5);
		ret();
	L(tblL);
		putL(incL);
		putL(addL);

		ready();
	}
};

int main(int argc, char *[])
{
	Code c(argc > 1);
	puts("start");
	auto f = c.getCode<int (*)()>();
	int v = f();
	printf("v=%d\n", v);
	puts("end");
}

