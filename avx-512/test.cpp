#include <stdio.h>
#include <memory.h>
#include <stdint.h>
#include <string>

extern "C" {
void getmant0(float *, const float *x);
void getmant1(float *, const float *x);
void getmant2(float *, const float *x);
void getmant3(float *, const float *x);
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
			printf("e=%d m=%08x\n", e, m);
			float fi;
			fi = u2f(((e + 127) << 23) | m);
			putBin("inp", fi);
			for (int mode = 0; mode < 4; mode++) {
				const pf f = fTbl[mode];
				float fo;
				f(&fo, &fi);
				putBin("out", fo);
				float emu = getmant_emu(fi, mode);
				if (emu != fo) {
					printf("ERR %f %f\n", emu, fo);
				}
			}
		}
	}
}

int main()
{
	getmant_test();
}

