#include <fjcex.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <emmintrin.h>

union ud {
	uint64_t u;
	double d;
};

_fjsp_v2r8 aaa(_fjsp_v2r8 a, _fjsp_v2r8 b) {
	return _fjsp_trissel_v2r8(a, b);
}

#if 1
_fjsp_v2r8 trissel(_fjsp_v2r8 a, _fjsp_v2r8 b)
{
	uint64_t ua[2];
	uint64_t ub[2];
	uint64_t out[2];
	ud ud;
	ud.d = 1.0;
	memcpy(ua, &a, sizeof(ua));
	memcpy(ub, &b, sizeof(ub));
	for (int i = 0; i < 2; i++) {
		uint64_t x = ub[i];
		if (x & 1) {
			x = ua[i];
		} else {
			x = ud.u;
		}
		x ^= uint64_t((x >> 1) & 1) << 63;
		out[i] = x;
	}
	_fjsp_v2r8 c;
	memcpy(&c, out, sizeof(c));
	return c;
}
#endif

void testTrissel()
{
	double dTbl[] = {
		1.0, 2.2, -3.4, 4.5, 0
	};
	int64_t uTbl[] = {
		0, 1, 2, 3, 0
	};
	for (size_t i = 0; i < sizeof(dTbl) / sizeof(*dTbl); i++) {
		double d[2] = { dTbl[i], dTbl[i] };
		for (size_t j = 0; j < sizeof(uTbl) / sizeof(*uTbl); j++) {
			int64_t u[2] = { uTbl[j], uTbl[j] };
			_fjsp_v2r8 a, b;
			memcpy(&a, d, sizeof(a));
			memcpy(&b, u, sizeof(b));
			_fjsp_v2r8 org, emu;
			org = _fjsp_trissel_v2r8(a, b);
			emu = trissel(a, b);
			double o1[2], o2[2];
			memcpy(o1, &org, sizeof(o1));
			memcpy(o2, &emu, sizeof(o2));
			printf("a=% 7.3e b=% 2ld ", d[0], u[0]);
			printf("org % 7.3e, emu % 7.3e(%c)\n", o1[0], o2[0], o1[0] == o2[0] ? 'o' : 'x');
		}
	}
}

int main()
{
//	testTrismul();
	testTrissel();
}
