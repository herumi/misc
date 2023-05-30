#include <stdio.h>

extern "C" void func(float *x, const float *y, const float *z);

int main()
{
	float x = 2;
	float y = 3;
	float z = 4;
	func(&x, &y, &z);
	printf("x=%f\n", x);
}
