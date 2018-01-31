#include <stdio.h>
#include <initializer_list>

void put(const std::initializer_list<int>& v)
{
	for (int x : v) {
		printf("%d ", x);
	}
	printf("\n");
}
int main()
{
//	auto x1 = {1};
//	put(x1);
	auto x2 {2, 3};
	put(x2);
}
