#include "var.hpp"

void put();
void incX();

int main()
{
	printf("msg=%p\n", X<int>::get());
	put();
	incX();
	incX();
	X<int>().inc();
	printf("main x=%d\n", X<int>::x);
}
