#include <stdio.h>

extern "C" int fff(int, int);

int main()
{
	printf("%d + %d = %d\n", 3, 5, fff(3, 5));
}
