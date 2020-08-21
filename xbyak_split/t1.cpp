#include <xbyak/xbyak.h>
#include <stdio.h>

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		mov(eax, 123);
		ret();
	}
};

int main()
{
	Code c;
	int (*f)() = c.getCode<int (*)()>();
	printf("f=%d\n", f());
}

