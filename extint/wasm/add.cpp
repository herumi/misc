#include "api.h"
#include "../exti.hpp"

inline exti256_t* cast256(int *x) { return (exti256_t*)x; }
inline const exti256_t* cast256(const int *x) { return (const exti256_t*)x; }
inline exti512_t* cast512(int *x) { return (exti512_t*)x; }
inline const exti512_t* cast512(const int *x) { return (const exti512_t*)x; }

extern "C" {

API void add256(int *z, const int *x, const int *y)
{
	*cast256(z) = *cast256(x) + *cast256(y);
}

}

