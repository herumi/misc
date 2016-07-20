/*
	g++ -std=c++11 -g -pthread dead-lock.cpp
*/
#include <thread>
#include <mutex>
#include <stdio.h>

void f(std::mutex& m)
{
	puts("f");
	puts("try lock m");
	std::lock_guard<std::mutex> lk(m);
	puts("locked m");
}

int main()
	try
{
	std::mutex m;
	m.lock();
	std::thread t(f, std::ref(m));
	t.join();
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
