#include <stdio.h>

extern "C" int sum(int);
extern "C" int sum2(int);
extern "C" int test1(int);

int main()
{
	for (int i = 1; i < 11; i++) {
		printf("%d %d %d\n", i, sum(i), sum2(i));
	}
	for (int i = 2; i < 6; i++) {
		printf("%d %d\n", i, test1(i));
	}
}
