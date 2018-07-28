#include <stdio.h>

extern "C" int add(int x, int y);

int main()
{
	printf("%d\n", add(3, 5));
}
