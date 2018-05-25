#include <stdio.h>
#include <stdlib.h>
#include <thread>
#ifdef _MSC_VER
	#include <intrin.h>
#else
	#include <x86intrin.h>
#endif

static void lock(volatile int *my, volatile int *other, volatile int *turn, int other_id)
{
	*my = 1;
	*turn = other_id;
	_mm_mfence();
	while (*other && *turn == other_id) {
	}
}

static void unlock(volatile int *my)
{
	*my = 0;
}

const int COUNT = 5000000;
static volatile int counter = 0;

void task(volatile int *my, volatile int *other, volatile int *turn, int id)
{
	for (int i = 0; i < COUNT; i++) {
		lock(my, other, turn, id);
		counter++;
		unlock(my);
	}
}

int main()
{
	{
		volatile int f0 = 0;
		volatile int f1 = 0;
		volatile int turn = 0;
		std::thread t0(task, &f0, &f1, &turn, 0);
		std::thread t1(task, &f1, &f0, &turn, 1);
		t0.join();
		t1.join();
	}

	printf("counter = %d\n", counter);
	printf("difference = %d\n", COUNT * 2 - counter);
}
