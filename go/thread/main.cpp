#include <stdio.h>
#include <stdlib.h>

extern "C" void waitTask(int n);

int main(int argc, char *argv[])
{
	int n = argc == 1 ? 8 : atoi(argv[1]);
	printf("main n=%d\n", n);
	waitTask(n);
	puts("main end");
}
