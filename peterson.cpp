#include <stdio.h>
#include <stdlib.h>
#include <cybozu/thread.hpp>

static volatile int f0 = 0;
static volatile int f1 = 0;
static volatile int turn;

static void lock(volatile int *my_flag, volatile int *other_flag, int other_id)
{
	*my_flag = 1;
	turn = other_id;
//	_mm_mfence();
	while (*other_flag && turn == other_id) {
	}
}

static void unlock(volatile int *my_flag)
{
	*my_flag = 0;
}

const int COUNT = 5000000;
static int counter = 0;

void task0()
{
#if 0
	for (int i = 0; i < COUNT; i++) {
		f0 = 1;
		turn = 1;
		_mm_mfence();
		while (f1 && turn == 1);
		counter++;
		f0 = 0;
	}
#else
	for (int i = 0; i < COUNT; i++) {
		lock(&f0, &f1, 1);
		counter++;
		unlock(&f0);
	}
#endif
}

void task1()
{
#if 0
	for (int i = 0; i < COUNT; i++) {
		f1 = 1;
		turn = 0;
		_mm_mfence();
		while (f0 && turn == 0);
		counter++;
		f1 = 0;
	}
#else
	for (int i = 0; i < COUNT; i++) {
		lock(&f1, &f0, 0);
		counter++;
		unlock(&f1);
	}
#endif
}

struct Thread : cybozu::ThreadBase {
	void (*t_)();
	Thread(void (*t)()) : t_(t) {}
	void threadEntry()
	{
		t_();
	}
};

int main()
{
	{
		Thread t0(task0);
		Thread t1(task1);
		t0.beginThread();
		t1.beginThread();
		t0.joinThread();
		t1.joinThread();
	}

	printf("counter = %d\n", counter);
	printf("difference = %d\n", COUNT * 2 - counter);
}
