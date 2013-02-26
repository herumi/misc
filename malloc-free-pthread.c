/*
	gcc malloc-free-pthread.c -lpthread -Wall -Wextra -ansi -pedantic -O2
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>

void task(int idx)
{
	const int size = 100;
	int i;
	for (i = 1; i < size; i++) {
		char *p = (char*)malloc(i);
		memset(p, idx, i);
		free(p);
	}
}

typedef struct {
	int begin;
	int end;
} Range;

void* run(void *arg)
{
	const Range *range = (const Range*)arg;
	int begin = range->begin;
	int end = range->end;
	printf("begin=%d, end=%d\n", begin, end);
	while (begin != end) {
		task(begin);
		begin++;
	}
	return 0;
}

#define MAX_THREAD_NUM 4

int main(int argc, char *argv[])
{
	const int threadNum = argc == 1 ? 1 : atoi(argv[1]);
	const int n = 1 * 2 * 3 * 4 * 5000;
	if (threadNum < 0 || threadNum > MAX_THREAD_NUM) {
		printf("threadNum = 0, 1, 2, 3, 4\n");
		return 1;
	}
	printf("threadNum=%d, n=%d\n", threadNum, n);
	if (threadNum == 0) {
		Range range;
		puts("no thread\n");
		range.begin = 0;
		range.end = n;
		run(&range);
	} else {
		const int dn = n / threadNum;
		Range range[MAX_THREAD_NUM];
		pthread_t pt[MAX_THREAD_NUM];
		int i;
		for (i = 0; i < threadNum; i++) {
			range[i].begin = i * dn;
			range[i].end = (i + 1) * dn;
			if (pthread_create(&pt[i], NULL, run, &range[i]) != 0) {
				printf("ERR create %d\n", i);
				return 1;
			}
		}
		for (i = 0; i < threadNum; i++) {
			if (pthread_join(pt[i], NULL) != 0) {
				printf("ERR join %d\n", i);
				return 1;
			}
		}
	}
	puts("end");
	return 0;
}
