#include <xbyak/xbyak_util.h>

struct AddNC : Xbyak::CodeGenerator {
	AddNC()
	{
		using namespace Xbyak;
		util::StackFrame sf(this, 3);
		const Reg64& z = sf.p[0];
		const Reg64& x = sf.p[1];
		const Reg64& y = sf.p[2];
		mov(rax, ptr [x]);
		add(rax, ptr [y]);
		mov(ptr [z], rax);
		for (int i = 1; i < 3; i++) {
			mov(rax, ptr [x + i * 8]);
			adc(rax, ptr [y + i * 8]);
			mov(ptr [z + i * 8], rax);
		}
	}
};

int main()
{
	AddNC code;
	void (*addNC)(uint64_t*, const uint64_t*, const uint64_t*) = code.getCode<void (*)(uint64_t*, const uint64_t*, const uint64_t*)>();
	uint64_t x[4], y[4], z[4];
	addNC(z, x, y);
}
