#include <stdio.h>
#include <thread>
#include <mutex>
#include <atomic>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>


const int N = 10000000;

void loop(void f())
{
	for (int i = 0; i < N; i++) {
		f();
	}
}

int ni;
void none_inc() { ni++; }
int none_get() { return ni; }

std::atomic<int> ai;
void atomic_inc() { ai++; }
int atomic_get() { return (int)ai; }

std::atomic<int> bi;
void atomic_inc2() { bi++; }

int mi;
std::mutex m;
void mutex_inc()
{
	std::lock_guard<std::mutex> al(m);
	mi++;
}
int mutex_get() { return mi; }
int mi2;
std::mutex m2;
void mutex_inc2()
{
	std::lock_guard<std::mutex> al(m2);
	mi2++;
}

void bench(const char *msg, void f(), int get(), void f2() = 0)
{
	Xbyak::util::Clock clk;
	clk.begin();
	std::thread t1(loop, f);
	std::thread t2(loop, f2 ? f2 : f);
	t1.join();
	t2.join();
	clk.end();
	printf("%s=%d\n", msg, get());
	printf("clk=%f\n", clk.getClock() / double(N));
}


int main()
{
	bench("none  ", none_inc, none_get);
	bench("atomic", atomic_inc, atomic_get);
	bench("mutex ", mutex_inc, mutex_get);
	bench("atomic not conflict", atomic_inc, atomic_get, atomic_inc2);
	bench("mutex  not conflict", mutex_inc, mutex_get, mutex_inc2);
}
