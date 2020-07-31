#include <atomic>

void store0(int& x)
{
	x = 5;
}

int load0(const int& x)
{
	return x;
}

void store1(std::atomic<int>& x)
{
	x = 3;
}

int load1(const std::atomic<int>& x)
{
	return x;
}

void store2(std::atomic<int>& x)
{
	x.store(7, std::memory_order_release);
}

int load2(const std::atomic<int>& x)
{
	return x.load(std::memory_order_acquire);
}

