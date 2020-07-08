#include <stdio.h>
#include "fexpa.hpp"

union fi {
	float f;
	uint32_t i;
};

float expC(float x)
{
	static const float log2_e = float(1 / log(2.0));
	float y = x * log2_e;
	int n = (int)floor(y);
	float a = y - n; // 0 <= a < 1
	n--;
	float b = 1 + a; // 1 <= b < 2, y = n + b
	fi remain;
	fi fi;
	fi.f = b;
	remain.i = (fi.i & FexpaTbl::mask(17)) | (127 << 23);
	fi.i >>= 17;
//printf("b=%f, L=%f R=%f, L+R=%f\n", b, fi.f, remain.f, fi.f + remain.f);
	float c = fexpaEmu(fi.f);
	return powf(2, (float)n) * c * pow(2, remain.f);
}

int main()
{
	for (float x = 0; x < 2; x += 0.1) {
		float y1 = expf(x);
		float y2 = expC(x);
		float e = fabsf(y1 - y2);
		printf("x=%f, y1=%f, y2=%f, diff=%e\n", x, y1, y2, e);
	}
}

