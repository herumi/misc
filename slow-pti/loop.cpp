#include <stdio.h>
#include <time.h>

void add(long *x, const long *y)
{
	x[0] += x[1] + y[0];
	x[1] += y[1] + x[0];
}

int main(int argc, char *[])
{
	long x[2] = { argc, argc };
	long y[2] = { argc, argc };
	void (*f)(long *x, const long *y);
	if (argc) f = add;
	clock_t begin, end;
	begin = clock();
	for (int i = argc; i < 1000000000; i++) {
		x[0] += x[1] + y[0];
		x[1] += y[1] + x[0];
	}
	end = clock();
	printf("inline %ld %ld %f\n", x[0], x[1], (end - begin) / double(CLOCKS_PER_SEC));
	begin = clock();
	for (int i = argc; i < 1000000000; i++) {
		add(x, y);
	}
	end = clock();
	printf("call fct %ld %ld %f\n", x[0], x[1], (end - begin) / double(CLOCKS_PER_SEC));
	begin = clock();
	for (int i = argc; i < 1000000000; i++) {
		f(x, y);
	}
	end = clock();
	printf("call fct ptr %ld %ld %f\n", x[0], x[1], (end - begin) / double(CLOCKS_PER_SEC));
}

