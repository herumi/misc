#include <stdio.h>

size_t getSp()
{
    size_t sp;
    __asm__ volatile("mov %%rsp, %0" : "=r"(sp));
    return sp;
}

int main()
{
	size_t sp1 = getSp();
	printf("sp1=%zx\n", sp1);
	int sp2 = sp1 - 0x1000;
	__asm__ volatile("mov %0, %%esp":"=r"(sp2));
	size_t sp3 = getSp();
	printf("sp2=%x\n", sp2);
	printf("sp3=%zx\n", sp3);
}
