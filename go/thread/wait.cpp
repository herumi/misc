#include <thread>
#include <chrono>
#include <iostream>
#include <stdio.h>

void put(int i)
{
	std::cout << "thead=" << std::this_thread::get_id() << " i=" << i << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
}
extern "C" void waitTask(int n)
{
	const int N = 8;
	std::thread th[N];
	if (n > N) n = N;
	for (int i = 0; i < n; i++) {
		th[i] = std::thread(put, i);
	}
	for (int i = 0; i < n; i++) {
		th[i].join();
	}
	puts("end");
}

