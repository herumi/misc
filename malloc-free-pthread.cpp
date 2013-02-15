#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <vector>
#include <pthread.h>

void task(int idx)
{
	const int size = 100;
	for (int i = 1; i < size; i++) {
		char *p = (char*)malloc(i);
		memset(p, idx, i);
		free(p);
	}
}

struct Range {
	int begin;
	int end;
};

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

int main(int argc, char *argv[])
{
	const int threadNum = argc == 1 ? 1 : atoi(argv[1]);
	const int maxThreadNum = 4;
	if (threadNum < 0 || threadNum > maxThreadNum) {
		printf("threadNum = 0, 1, 2, 3, 4\n");
		return 1;
	}
	const int n = 1 * 2 * 3 * 4 * 10000;
	printf("threadNum=%d, n=%d\n", threadNum, n);
	if (threadNum == 0) {
		puts("no thread\n");
		Range range;
		range.begin = 0;
		range.end = n;
		run(&range);
	} else {
		const int dn = n / threadNum;
		std::vector<Range> range(threadNum);
		std::vector<pthread_t> pt(threadNum);
		for (int i = 0; i < threadNum; i++) {
			range[i].begin = i * dn;
			range[i].end = (i + 1) * dn;
			if (pthread_create(&pt[i], NULL, run, &range[i]) != 0) {
				printf("ERR create %d\n", i);
				return 1;
			}
		}
		for (int i = 0; i < threadNum; i++) {
			if (pthread_join(pt[i], NULL) != 0) {
				printf("ERR join %d\n", i);
				return 1;
			}
		}
	}
	puts("end");
}
