#include "var.hpp"

int main()
{
	printf("msg=%p\n", X<int>::get());
	put();
	incX();
	incX();
	X<int>().inc();
	int x = X<int>::x;
	printf("main x=%d(%s)\n",x, x == 8 ? "ok" : "ng");
	printf("XX::x=%d\n", XX::x);
}
