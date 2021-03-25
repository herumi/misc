#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak_aarch64;

void put(const void *p)
{
	printf("p=%p\n", p);
}

struct Code : CodeGenerator {
	Code()
	{
		stp(x29, x30, pre_ptr(sp, -32));

		const size_t addr = (size_t)put;
		const size_t mask = 0xffff;
		mov(x0, (addr >> 48) & mask);
		lsl(x0, x0, 16);
		mov(x1, (addr >> 32) & mask);
		orr(x0, x0, x1);
		lsl(x0, x0, 16);
		mov(x1, (addr >> 16) & mask);
		orr(x0, x0, x1);
		lsl(x0, x0, 16);
		mov(x1, (addr >> 0) & mask);
		orr(x0, x0, x1);
		blr(x0);

		ldp(x29, x30, post_ptr(sp, 32));
		ret();
	}
};

int main()
{
	Code c;
	c.ready();
	auto f = c.getCode<void (*)()>();
	printf("put=%p\n", put);
	f();
}

