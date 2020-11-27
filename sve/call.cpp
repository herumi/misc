#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak_aarch64;

void put(int x)
{
	printf("x=%d\n", x);
}

struct Code : CodeGenerator {
	/*
		void f(int x, void put(int))
		{
			put(x);
			put(x + 5);
		}
	*/
	Code()
	{
		const auto& x = x19;
		const auto& func = x20;
		stp(x29, x30, pre_ptr(sp, -32));
		stp(x, func, ptr(sp, 16));

		mov(x, x0);
		mov(func, x1);

		blr(func);

		mov(x0, x);
		add(x0, x0, 5);

		blr(func);

		ldp(x, func, ptr(sp, 16));
		ldp(x29, x30, post_ptr(sp, 32));
		ret();
	}
};

int main()
{
	Code c;
	c.ready();
	auto f = c.getCode<void (*)(int, void put(int))>();
	f(123, put);
}

