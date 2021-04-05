#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <string.h>

union fi {
	uint32_t i;
	float f;
};

struct FloatFormat {
    uint32_t f:23;
    uint32_t e:8;
    uint32_t s:1;
	FloatFormat(uint32_t s = 0, uint32_t e = 0, uint32_t f = 0)
		: f(f)
		, e(e)
		, s(s)
	{
	}
	void set(uint32_t s_, uint32_t e_, uint32_t f_)
	{
		s = s_;
		e = e_;
		f = f_;
	}
	void put() const
	{
		fi fi;
		memcpy(&fi, this, sizeof(*this));
		printf("%08x %d %f %e\n", fi.i, fpclassify(fi.f), fi.f, fi.f);
	}
};


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

