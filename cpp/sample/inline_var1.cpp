#include "inline_var.hpp"

void put1()
{
	puts("put1");
	printf("static inline aaa=%d (%p)\n", aaa, &aaa);
	printf("       inline bbb=%d (%p)\n", bbb, &bbb);
	printf("X<int>::aaa      =%d (%p)\n", X<int>::aaa, &X<int>::aaa);
}

extern void put2();

int main()
{
	put1();
	put2();
}
