#include <stdio.h>
#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/test.hpp>

extern "C" {

void gen_fma(float *x, const float *y, const float *z);
void gen_mov(float *y, const float *x);
void gen_movq(float *y, const float *x, int mode);
void gen_vpalignr(uint8_t y[32], const uint8_t x[32]);
void loadf_avx(float *y, const float *x, size_t n);

}

void put(const void *p_, size_t n, const char *msg = "")
{
	if (msg) printf("%s ", msg);
	const uint8_t *p = (const uint8_t*)p_;
	for (size_t i = 0; i < n; i++) {
		printf("%02x ", p[i]);
	}
	printf("\n");
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

void test_vpalignr()
{
	uint8_t x[64];
	uint8_t y[64];
	for (int i = 0; i < 64; i++) {
		x[i] = uint8_t(i+1);
	}
	put(x, 64, "x");
	gen_vpalignr(y, x);
	put(y, 64, "y");

}

void test_loadf_avx()
{
	const size_t N = 8;
	const float boundary = 123456;
	float inp[N], out[N+1];
	for (int n = 1; n <= 8; n++) {
		printf("n=%d\n", n);
		for (int i = 0; i < n; i++) {
			inp[i] = (i+10) * 0.1f;
		}
		out[n] = boundary;
		loadf_avx(out, inp, n);
		for (int i = 0; i < n; i++) {
//			printf("%d inp %f out %f %d\n", i, inp[i], out[i], inp[i]*2 == out[i]);
		}
		for (int i = 0; i < n; i++) {
			CYBOZU_TEST_EQUAL(out[i], inp[i]*2);
		}
		CYBOZU_TEST_EQUAL(out[n], boundary);
	}
}

void test_movq()
{
	const float x[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	for (int mode = 0; mode < 2; mode++) {
		printf("mode=%d\n", mode);
		float y[8] = {};
		gen_movq(y, x, mode);
		printf("y=");
		for (int i = 0; i < 8; i++) {
			printf("%f ", y[i]);
		}
		printf("\n");
	}
}

int main()
{
	test_fma();
	test_mov();
	test_movq();
	test_vpalignr();
	test_loadf_avx();
}
