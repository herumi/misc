#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include "../../mcl/src/avx512.hpp"

extern "C" {

void select0(Vec *z, const Vec *x, const Vec *y);
void select1(Vec *z, const Vec *x, const Vec *y);
void misc(Vec *z, const Vec *x, const Vec *y);

}

const size_t N = 8;

void dump(const Vec& x, const char *msg = nullptr)
{
	const uint8_t *p = (const uint8_t*)&x;
	if (msg) {
		printf("%s\n", msg);
	}
	for (size_t i = 0; i < sizeof(x); i++) {
		printf("%02x", p[i]);
		if ((i & 7) == 7) putchar(' ');
	}
	printf("\n");
}

void set(Vec& x, const uint64_t a[N])
{
	memcpy(&x, a, sizeof(x));
}

int main()
{
	Vec z, x, y;
	uint64_t ax[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	uint64_t ay[] = { 0, 0, 0, 0, 0, 5, 0, 0, 0 };
	set(x, ax);
	set(y, ay);
	misc(&z, &x, &y);
	dump(z, "z");
}
