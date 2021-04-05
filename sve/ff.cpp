#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "floatformat.hpp"

int main()
{
	printf("sizeof ff=%zd\n", sizeof(FloatFormat));
	const struct {
		uint32_t s;
		uint32_t e;
		uint32_t f;
	} tbl[] = {
		{ 0, 0, 0 },
		{ 0, 127, 0 },
		{ 0, 0, 1 },
		{ 0, 128, 0 },
		{ 0, 255, 0 },
		{ 0, 255, 1 },
		{ 0, 255, 1 << 22 },
		{ 0, 255, 1 << 21 },
	};
	printf("NAN=%d\n", FP_NAN);
	printf("INFINITE=%d\n", FP_INFINITE);
	printf("ZERO=%d\n", FP_ZERO);
	printf("SUBNORMAL=%d\n", FP_SUBNORMAL);
	printf("NORMAL=%d\n", FP_NORMAL);
	FloatFormat ff;
	for (size_t i = 0; i < sizeof(tbl)/sizeof(tbl[0]); i++) {
		uint32_t s = tbl[i].s;
		uint32_t e = tbl[i].e;
		uint32_t f = tbl[i].f;
		ff.set(s, e, f);
		ff.put();
		ff.set(1 - s, e, f);
		ff.put();
	}
}

