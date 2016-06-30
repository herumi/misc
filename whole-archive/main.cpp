#include <stdio.h>

extern "C" void sub();

int main()
{
	puts("call sub");
	sub();
}
