#include <utility>
#include <array>
#include <mutex>
#include <memory>

int main()
{
	std::pair<int, double> p1 = { 234, 4.2 };
	std::array<int, 4> p2 = { 1, 3, 4, 2 };
	std::mutex m;
	std::lock_guard<std::mutex> lk1(m);
#ifdef CPP17
	std::pair q1 = { 234, 4.2 };
	std::array q2 = { 1, 3, 4, 2 };
	std::lock_guard lk2(m);
#endif
}
