/*
g++ -g calling.cpp verify_stack_frame.cpp -finstrument-functions && ./a.out
*/
#include <stdio.h>
#include "verify_stack_frame.hpp"

void k()
{
	puts("k");
}

void h()
{
	puts("h");
}
void g()
{
	puts("g");
	h();
}
void f()
{
	puts("f");
	g();
}

int main()
{
	puts("main");
	f();
}

