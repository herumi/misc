#include <xbyak_aarch64/xbyak_aarch64.h>

struct Code : public Xbyak_aarch64::CodeGenerator {
	typedef Xbyak_aarch64::ZReg ZReg;
	typedef Xbyak_aarch64::PReg PReg;
	Code()
	{
		mov(w0, ~0x1ffff);
		ret();
	}
};

int main()
	try
{
	Code c;
	c.ready();
	int (*f)() = c.getCode<int (*)()>();
	printf("f=%08x\n", f());
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}
