#include <stdio.h>
#include <stdint.h>

uint32_t neg(uint32_t a, uint32_t mask)
{
	return (a ^ mask) - mask;
}

uint32_t mul(uint32_t a, uint32_t b)
{
	uint32_t sa = int(a) >> 31;
	uint32_t sb = int(b) >> 31;
	uint32_t aa = neg(a, sa);
	uint32_t bb = neg(b, sb);
	aa *= bb;
	sa ^= sb;
	return neg(aa, sa);
}

int main()
{
	int a = 9;
	int b = 50;
	uint32_t a1 = a;
	uint32_t a2 = -a;
	uint32_t b1 = b;
	uint32_t b2 = -b;
	uint32_t c11 = a1 * b1;
	uint32_t c12 = a1 * b2;
	uint32_t c21 = a2 * b1;
	uint32_t c22 = a2 * b2;
	printf("11 %08x x %08x = %08x (%08x) %d %d\n", a1, b1, c11, -c11, a * b, mul(a, b));
	printf("12 %08x x %08x = %08x (%08x) %d %d\n", a1, b2, c12, -c12, a * (-b), mul(a, -b));
	printf("21 %08x x %08x = %08x (%08x) %d %d\n", a2, b1, c21, -c21, (-a) * b, mul(-a, b));
	printf("22 %08x x %08x = %08x (%08x) %d %d\n", a2, b2, c22, -c22, (-a) * (-b), mul(-a, -b));
}

