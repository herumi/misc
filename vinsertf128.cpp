#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>

MIE_ALIGN(32) char buf[512];

double *u = (double*)(buf + 1);

MIE_ALIGN(32) double a[4];

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		mov(rax, (size_t)u);
		vmovups(xm0, ptr [rax]);
		vinsertf128(ym0, ym0, ptr [rax + 16], 1);
		mov(rax, (size_t)a);
		vmovaps(ptr [rax], ym0);
		ret();
	}
} code;

void (*f)() = code.getCode<void (*)()>();

int main()
	try
{
	u[0] = 1;
	u[1] = 2;
	u[2] = 3;
	u[3] = 5;
	f();
	printf("%f %f %f %f\n", a[3], a[2], a[1], a[0]);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}
