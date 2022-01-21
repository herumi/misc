#include <xbyak/xbyak.h>

using namespace Xbyak;

void putVal(int x)
{
	printf("x=%d\n", x);
}

struct Code : CodeGenerator {
	Code(bool stop)
	{
		Label clearEaxL, incEaxL, putValL;
		if (stop) int3();
		const Label* tbl[] = {
			&clearEaxL,
			&incEaxL,
			&putValL,
		};
		const size_t n = sizeof(tbl)/sizeof(tbl[0]);
		for (size_t i = 0; i < n; i++) {
			mov(rax, *tbl[n - 1 - i]);
			push(rax);
		}
		// exec
		ret();

	L(incEaxL);
		inc(eax);
		ret();
	L(clearEaxL);
		xor_(eax, eax);
		ret();
	L(putValL);
#ifdef XBYAK64_GCC
		mov(rdi, rax);
#else
		mov(rcx, rax);
#endif
		mov(rax, (size_t)putVal);
		call(rax);
		ret();
	}
};

int main(int argc, char *[])
{
	Code c(argc > 1);
	auto f = c.getCode<void (*)()>();
	f();
	puts("end");
}

