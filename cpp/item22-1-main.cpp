#include "item22-1.hpp"

int main()
{
	{
		puts("A");
		A a0;
		a0.put();
		A a1 = a0;
		a1.put();
		A a2(std::move(a0));
		a0.put();
		a2.put();
		A a3(a0);
		a3.put();
		a0 = a1;
		a0.put();
	}
	{
		puts("B");
		B a0;
		a0.put();
		B a1 = a0;
		a1.put();
		B a2(std::move(a0));
		a0.put();
		a2.put();
		B a3(a0);
		a3.put();
		a0 = a1;
		a0.put();
	}
}
