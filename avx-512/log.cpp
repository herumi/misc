#include <stdio.h>
#include <math.h>
#include <stdint.h>

union fi {
	float f;
	uint32_t i;
};

float logfC(float x)
{
	fi fi;
	fi.f = x;
	float e = (fi.i >> 23) - 127;
	fi.i = (fi.i & 0x7fffff) | (127 << 23);
	float y = fi.f;
	/*
		x = y * 2^e (1 <= y < 2)
		log(x) = e log2 + log y
		a = (y - sqrt(2)) / (y + sqrt(2))
		|a| <= (sqrt(2) - 1)/(sqrt(2) + 1)
		y = sqrt(2) (1+a)/(1-a)
		log(x) = e log2 + 1/2 log 2 + log((1+a)/(1-a))
		log((1+a)/(1-a)) = 2a(1 + a^2/3 + a^4/5 + a^6/7)
		b = a^2
		log(x) = (e+1/2) log2 + 2a(1 + b(1/3 + b(1/5 + b/7)))
	*/
	const float log2 = log(2.0f);
	const float log2div2 = log2 / 2;
	const float sqrt2 = sqrt(2.0f);
	const float coeff[] = {
		1.0f,
		1.0f / 3,
		1.0f / 5,
		1.0f / 7
	};
	float a = (y - sqrt2) / (y + sqrt2);
	e = log2 * e + log2div2;
	float b = a * a;
	x = coeff[3];
	x = b * x + coeff[2];
	x = b * x + coeff[1];
	x = b * x + coeff[0];
	x *= a;
	x += x;
	x += e;
	return x;
}

int main()
{
	for (float x = 0.5; x < 4; x += 0.5) {
		float y1 = log(x);
		float y2 = logfC(x);
		printf("x=%f diff=%e %e %e\n", x, std::abs(y1 - y2), y1, y2);
	}
}
