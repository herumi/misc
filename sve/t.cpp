#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak_aarch64;

struct Code : public CodeGenerator {
	Code()
	{
		fadd(z0.s, p0/T_m, z0.s);
		ret();
	}
};

int main()
	try
{
	Code c;
	c.ready();
	const uint32_t* d = c.getCode<uint32_t*>();
	printf("d=%08x\n", *d);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}
