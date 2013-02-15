// g++ -O3 -fopenmp t.cpp
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <omp.h>

void task(int idx)
{
	const int size = 100;
	for (int i = 1; i < size; i++) {
		char *p = (char*)malloc(i);
		memset(p, idx, i);
		free(p);
	}
}

int main(int argc, char *argv[])
{
	const int threadNum = argc == 1 ? 1 : atoi(argv[1]);
	omp_set_num_threads(threadNum);
	printf("thread %d\n", omp_get_max_threads());
	const int n = 1 * 2 * 3 * 4 * 1000;

	#pragma omp parallel for
	for (int i = 0; i < n; i++) {
		task(i);
	}
	puts("ok");
}

