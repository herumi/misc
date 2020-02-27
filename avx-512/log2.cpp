#include <stdio.h>
#include <math.h>
#include <stdint.h>

union fi {
	float f;
	uint32_t i;
};

float sign(int n)
{
	if (n > 0) return 1;
	return -1;
}
float logfC(float x)
{
	fi fi;
	fi.f = x;
	float e = (int(fi.i - (127 << 23))) >> 23;
	fi.i = (fi.i & 0x7fffff) | (127 << 23);
	float y = fi.f;
	/*
		x = y * 2^e (1 <= y < 2)
		log(x) = e log2 + log(y)
		a = (2/3) y - 1 (|a|<=1/3)
		y = 1.5(1 + a)
		log(y) = log 1.5 + log(1 + a)
		log(x) = e log2 + log 1.5 + (a - a^2/2 + a^3/3 - ...)
	*/
	const float log2 = log(2.0f);
	const float log1p5 = log(1.5f);
	float a = 2.0f/3 * y - 1;
	const int logN = 11;

	// remez(log(1+x),11,[-1/3,1/3]);
	float tbl[logN] = {
		 0.99999999876779161876,
		-0.50000006794862287478,
		 0.33333360042859534218,
		-0.24999295329269842009,
		 0.19998318415252753452,
		-0.16693231405697749195,
		 0.14331479724379799457,
		-0.12052778021731317453,
		 0.10506620003814258239,
		-0.13365969364887611568,
		 0.12828205069103372686,
	};
	x = tbl[logN - 1];
	for (int i = logN - 2; i >= 0; i--) {
		x = x * a + tbl[i];
	}
	e = e * log2 + log1p5;
	x = x * a + e;
	return x;
}

int main()
{
	double maxe = 0;
	double sum = 0;
	int count = 0;
	for (float x = 0.1; x <= 2; x += 1e-6) {
		float y1 = log(x);
		float y2 = logfC(x);
		double d = abs(y1 - y2);
//		printf("x=%e, y1=%e, y2=%e, diff=%e\n", x, y1, y2, d);
		sum += d;
		if (d > maxe) {
			maxe = d;
		}
		count++;
	}
	printf("maxe=%e, sum=%e, ave=%e\n", maxe, sum, sum / count);
}
