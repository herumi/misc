#include <stdio.h>
#include <memory.h>
#include <stdint.h>
#include <string>

extern "C" {
void getmant0(float *, const float *x);
void getmant1(float *, const float *x);
void getmant2(float *, const float *x);
void getmant3(float *, const float *x);

/*
out[0] = vrndscaleps(x)
out[1] = vreduceps(x)
*/
void vreduceps(float out[2], const float *x);
}

uint32_t f2u(float f)
{
	uint32_t u;
	memcpy(&u, &f, 4);
	return u;
}

float u2f(uint32_t u)
{
	float f;
	memcpy(&f, &u, 4);
	return f;
}

std::string f2s(float f)
{
	uint32_t u = f2u(f);
	std::string s;
	for (int i = 0; i < 32; i++) {
		if (i == 1 || i == 9) s += ':';
		s += u & (uint32_t(1) << (31 - i)) ? '1' : '0';
	}
	return s;
}

uint32_t mask(uint32_t n)
{
	if (n == 32) return 0xffffffff;
	return (1u << n) - 1;
}

void putBin(const char *msg, float f)
{
	printf("%s %s %e\n", msg, f2s(f).c_str(), f);
}

void assertEqual(float x, float y, bool acceptMinusZero = false)
{
	uint32_t ux = f2u(x);
	uint32_t uy = f2u(y);
	// accept 0 == -0
	if (acceptMinusZero && (ux&0x7fffffff) == 0 && (uy&0x7fffffff) == 0) return;
	if (ux != uy) {
		printf("err x=%f(%08x) y=%f(%08x)\n", x, ux, y, uy);
		exit(1);
	}
}

int getBaseExp(int mode, int exp, uint32_t mant)
{
	switch (mode) {
	case 0: return 0;
	case 1: return (exp & 1) ? -1 : 0;
	case 2: return -1;
	case 3: return (mant >> 22) ? -1 : 0;
	default: return 0;
	}
}

/*
	f = 2^n a, n : integer, 1 <= a < 2
	mode = 0 : return a;
	mode = 1 : return n is even ? a/2 : a;
	mode = 2 : return a/2;
	mode = 3 : return (1.5 <= a) ? a/2 : a;
*/
float getmant_emu(float f, int mode)
{
	uint32_t u = f2u(f);
	uint32_t mant = u & mask(23);
	int exp = int((u >> 23) & mask(8)) - 127;
	exp = getBaseExp(mode, exp, mant);
	return u2f(((exp + 127) << 23) | mant);
}

void getmant_test()
{
	typedef void (*pf)(float*, const float*);
	puts("getmant_test");
	pf fTbl[] = {
		getmant0,
		getmant1,
		getmant2,
		getmant3,
	};
	const uint32_t mantTbl[] = {
		0,
		1,
		mask(23) >> 1,
		mask(23) - 1,
		mask(23),
		0x1234,
		0x12345,
	};
	for (int e = -3; e < 3; e++) {
		for (uint32_t m : mantTbl) {
//			printf("e=%d m=0x%08x\n", e, m);
			float fi;
			fi = u2f(((e + 127) << 23) | m);
//			putBin("inp", fi);
			for (int mode = 0; mode < 4; mode++) {
				const pf f = fTbl[mode];
				float fo;
				f(&fo, &fi);
//				putBin("out", fo);
				float emu = getmant_emu(fi, mode);
				assertEqual(emu, fo);
			}
		}
	}
}

void vreduce_test_one(uint32_t s, uint32_t e, uint32_t m)
{
	uint32_t u = (s << 31) | (e << 23) | m;
	float f = u2f(u);
	float out[2];
	vreduceps(out, &f);
//	printf("s=%d e=%d m=0x%08x\n", s, e, m);
//	printf("s=%d e=%d f=%e out[0]=%e out[1]=%e\n", s, e, f, out[0], out[1]);
	assertEqual(out[0] + out[1], f, true);
	assertEqual(f - out[0], out[1], true);
	assertEqual(f - out[1], out[0], true);
}

void vreduce_test()
{
	puts("vreduce_test");
	uint32_t mTbl[] = { 0, 1, mask(23) };
	for (size_t i = 0; i < sizeof(mTbl)/sizeof(mTbl[0]); i++) {
		uint32_t m = mTbl[i];
		// skip NaN(e=255)
		for (int e = 0; e < 255; e++) {
			vreduce_test_one(0, e, m);
			vreduce_test_one(1, e, m);
		}
	}
}

int main()
{
	vreduce_test();
	getmant_test();
	puts("ok");
}

