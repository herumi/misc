#include "inline_var.hpp"

void put2()
{
	puts("put2");
	printf("static inline aaa=%d (%p)\n", aaa, &aaa);
	printf("       inline bbb=%d (%p)\n", bbb, &bbb);
	printf("X<int>::aaa      =%d (%p)\n", X<int>::aaa, &X<int>::aaa);
}
