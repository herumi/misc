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
	float e = (fi.i - (127 << 23)) >> 23;
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
#if 0
		// maxe=1.072066e-07, sum=2.183438e-03, ave=2.186390e-08
		1.0f,
		1.0f / 3,
		1.0f / 5,
		1.0f / 7
#else
#if 1
		// maxe=8.293759e-08, sum=2.167962e-02, ave=2.067527e-08
		0.9999999968719069552145263001550,
		0.3333347422539786943017443341906,
		0.1998289745987578035122400619786,
		0.1505016409143539640686624507163,
#else
		// maxe=8.293759e-08, sum=2.169730e-02, ave=2.069214e-08
		// by Maple
		0.999999997712359975865210404442,
		0.333334469118408692463554581643,
		0.199850402410170522492027108176,
		0.150032678359085280259754602993,
#endif
#endif
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
	double maxe = 0;
	double sum = 0;
	int count = 0;
	for (float x = 1; x <= 2; x += 1e-6) {
		float y1 = log(x);
		float y2 = logfC(x);
		double d = abs(y1 - y2);
		sum += d;
		if (d > maxe) {
			maxe = d;
		}
		count++;
	}
	printf("maxe=%e, sum=%e, ave=%e\n", maxe, sum, sum / count);
}
