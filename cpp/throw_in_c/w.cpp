#include <stdio.h>
#include <stdexcept>

static void f_throw()
{
	puts("f_throw");
	throw std::runtime_error("f_throw");
}

void (*getCpp())()
{
	puts("getCpp");
	return f_throw;
}

