#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int n = argc == 2 ? atoi(argv[1]) : 12345;
	printf("time=%d\n", time(0));
	printf("n=%d\n", n);
	srand(n);
	for (int i = 0;i < 20; i++) {
		int a = rand();
		printf("%d ", a % 10);
	}
	printf("\n");
}
