#include <stdio.h>

extern "C" {

void gen_fma(float *x, const float *y, const float *z);
void gen_mov(float *y, const float *x);

}

void test_fma()
{
	puts("test_fma");
	float x = 2;
	float y = 3;
	float z = 4;
	gen_fma(&x, &y, &z);
	printf("x=%f\n", x);
}

void test_mov()
{
	puts("test_mov");
	float x[16];
	float y = 1.25f;
	gen_mov(x, &y);
	for (int i = 0; i < 16; i++) {
		printf("%2d %f\n", i, x[i]);
	}
}

int main()
{
	test_fma();
	test_mov();
}
