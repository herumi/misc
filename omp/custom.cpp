/*
	-fopenmp
*/
#include <stdio.h>
#include <omp.h>

struct X {
	int x;
	X() : x(0) {}
	void add(const X& other)
	{
		x += other.x;
	}
};

void aggregate(X& out, const X *x, size_t n)
{
	out.x = 0;
	#pragma omp declare reduction(Xadd: X: omp_out.add(omp_in)) initializer(omp_priv=omp_orig)
	#pragma omp parallel for reduction(Xadd:out)
	for (size_t i = 0; i < n; i ++) {
		out.add(x[i]);
	}
}

int main()
{
	X sum;
	const size_t n = 10000;
	X x[n];
	for (int i = 0; i < n; i++) {
		x[i].x = i;
	}
	aggregate(sum, x, n);
	printf("sum=%d\n", sum.x);
}
