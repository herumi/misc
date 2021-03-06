#pragma once
#include <stdint.h>
#ifndef __wasm__
#include <stdio.h>
#endif

typedef unsigned _ExtInt(256) exti256_t;
typedef unsigned _ExtInt(384) exti384_t;
typedef unsigned _ExtInt(512) exti512_t;
typedef unsigned _ExtInt(768) exti768_t;

#ifndef __wasm__
template<class T>
void dump(const T& x)
{
	const uint64_t *p = (const uint64_t *)&x;
	const size_t n = sizeof(x) / 8;
	for (size_t i = 0; i < n; i++) {
		printf("%016lx", p[n - 1 - i]);
	}
	printf("\n");
}
#endif

inline void exti_mul256(exti512_t& z, const exti256_t& x, const exti256_t& y)
{
	z = exti512_t(x) * exti512_t(y);
}

inline void exti_sqr256(exti512_t& y, const exti256_t& x)
{
	y = exti512_t(x) * exti512_t(x);
}

inline void exti_mul384(exti768_t& z, const exti384_t& x, const exti384_t& y)
{
	z = exti768_t(x) * exti768_t(y);
}

inline void exti_sqr384(exti768_t& y, const exti384_t& x)
{
	y = exti768_t(x) * exti768_t(x);
}

#ifndef __wasm__
void (*emul256)(exti512_t& z, const exti256_t& x, const exti256_t& y) = exti_mul256;
void (*esqr256)(exti512_t& y, const exti256_t& x) = exti_sqr256;
void (*emul384)(exti768_t& z, const exti384_t& x, const exti384_t& y) = exti_mul384;
void (*esqr384)(exti768_t& y, const exti384_t& x) = exti_sqr384;
#endif
