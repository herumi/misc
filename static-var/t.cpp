#include "var.hpp"

void put();

int main()
{
	printf("msg=%p\n", X<int>::get());
	put();
}
