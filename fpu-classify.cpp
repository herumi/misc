#include <stdio.h>
#include <math.h>
#include <float.h>

#ifdef _MSC_VER
const int FP_NAN = 0;
const int FP_INFINITE = 1;
const int FP_ZERO = 2;
const int FP_SUBNORMAL = 3;
const int FP_NORMAL = 4;

inline int fpclassify(double x)
{
	int c = _fpclass(x);
	if (c & (_FPCLASS_NN | _FPCLASS_PN)) return FP_NORMAL;
	if (c & (_FPCLASS_NZ | _FPCLASS_PZ)) return FP_ZERO;
	if (c & (_FPCLASS_ND | _FPCLASS_PD)) return FP_SUBNORMAL;
	if (c & (_FPCLASS_NINF | _FPCLASS_PINF)) return FP_INFINITE;
	// _FPCLASS_SNAN, _FPCLASS_QNAN
	return FP_NAN;
}
#endif

int main()
{
	for (int i = -711; i < -705; i++) {
		double a = exp(double(i));
		printf("a=%.18g %d\n", a, fpclassify(a) == FP_SUBNORMAL);
	}
	union di {
		unsigned long long i;
		double d;
	};
	const struct {
		unsigned long long i;
		int c;
		const char *msg;
	} tbl[] = {
		{ 0x0000000000000000ULL, FP_ZERO, "+zero" },
		{ 0x8000000000000000ULL, FP_ZERO, "-zero" },
		{ 0x3ff0000000000000ULL, FP_NORMAL, "+1" },
		{ 0xbff0000000000000ULL, FP_NORMAL, "-1" },
		{ 0x0010000000000000ULL, FP_NORMAL, "DBL_MIN" },
		{ 0x8010000000000000ULL, FP_NORMAL, "-DBL_MIN" },
		{ 0x7fefffffffffffffULL, FP_NORMAL, "DBL_MAX" },
		{ 0xffefffffffffffffULL, FP_NORMAL, "-DBL_MAX" },
		{ 0x000fffffffffffffULL, FP_SUBNORMAL, "large positive subnormal" },
		{ 0x800fffffffffffffULL, FP_SUBNORMAL, "large negative subnormal" },
		{ 0x0000000000000001ULL, FP_SUBNORMAL, "small positive subnormal" },
		{ 0x8000000000000001ULL, FP_SUBNORMAL, "small negative subnormal" },

		{ 0x7ff0000000000000ULL, FP_INFINITE, "+inf" },
		{ 0xfff0000000000000ULL, FP_INFINITE, "-inf" },
		{ 0x7ff0000000000001ULL, FP_NAN, "signaling nan" },
		{ 0x7ff7ffffffffffffULL, FP_NAN, "signaling nan" },
		{ 0xfff7ffffffffffffULL, FP_NAN, "-signaling nan" },
		{ 0x7ff8000000000000ULL, FP_NAN, "quiet nan" },
		{ 0xfff8000000000000ULL, FP_NAN, "quiet nan" },
		{ 0x7fffffffffffffffULL, FP_NAN, "quiet nan" },
		{ 0xffffffffffffffffULL, FP_NAN, "-quiet nan" },
	};
	for (size_t i = 0; i < sizeof(tbl) / sizeof(tbl[0]); i++) {
		di di;
		di.i = tbl[i].i;
		int c = fpclassify(di.d);
		printf("%016llx(%24.17g) %d(%s)", di.i, di.d, c, tbl[i].msg);
#ifdef _MSC_VER
		printf(" %x", _fpclass(di.d));
#endif
		if (c != tbl[i].c) printf(" ERR");
		printf("\n");
	}
}
