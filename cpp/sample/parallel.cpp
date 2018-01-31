#include <algorithm>
#include <numeric>
#include <execution>
#include <array>

int main()
{
	std::array a = { 1, 2,3,4, 5, 6, 7, 8 };
	int sum = std::reduce(std::execution::par_unseq, a.begin(), a.end());
	printf("sum=%d\n", sum);
}
