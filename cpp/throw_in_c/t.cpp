#include <stdio.h>
#include <stdexcept>

extern "C" void f();

extern "C" void g()
{
	throw std::runtime_error("g");
}

int main()
	try
{
	f();
} catch (std::exception& e){
	printf("e=%s\n", e.what());
}

