#include "fmath-sve.hpp"
#include <vector>
#include <time.h>

typedef std::vector<float> Fvec;

int main()
{
	Fvec x, y;
	const size_t n = 4096;
	const size_t C = 100000;
	x.resize(n);
	y.resize(n);
	for (size_t i = 0; i < n; i++) {
		x[i] = sin(i / double(n) * 7) * 10;
	}
	clock_t begin = clock();
	for (size_t i = 0; i < C; i++) {
		fmath::tanhf_v(&y[0], &x[0], n);
	}
	clock_t end = clock();
	printf("time=%.2fsec\n", (end - begin) / double(CLOCKS_PER_SEC));
	float maxe = 0;
	float maxx = 0;
	for (size_t i = 0; i < n; i++) {
		float e = std::fabs(y[i] - tanh(x[i]));
		if (e > maxe) {
			maxe = e;
			maxx = x[i];
		}
	}
	printf("maxe=%e, maxx=%e\n", maxe, maxx);
}
