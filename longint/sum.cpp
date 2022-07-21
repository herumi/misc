#include <stdio.h>

extern "C" int sum(int);
extern "C" int sum2(int);
extern "C" int test1(int);
extern "C" int test2(int);

int main()
{
	puts("sum");
	for (int i = 1; i < 11; i++) {
		printf("%d %d %d\n", i, sum(i), sum2(i));
	}
	puts("test1");
	for (int i = 2; i < 6; i++) {
		printf("%d %d\n", i, test1(i));
	}
	puts("test2");
	for (int i = 0; i < 4; i++) {
		printf("%d %d\n", i, test2(i));
	}
}
