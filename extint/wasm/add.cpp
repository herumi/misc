#include "api.h"
#include "../exti.hpp"

extern "C" int mulJS(int x, int y);

extern "C" {

API int add(int x, int y)
{
	return x + y;
}

API int callJS(int x, int y)
{
	return mulJS(x, y);
}

API int getPtr(int x)
{
	char buf[x];
	return (int)buf;
}

API void mul256(int *z, const int *x, const int *y)
{
	exti_mul256(*(exti512_t*)z, *(const exti256_t*)x, *(const exti256_t*)y);
}

}

