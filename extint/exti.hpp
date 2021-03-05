#pragma once
#include <stdint.h>
#include <stdio.h>

typedef unsigned _ExtInt(256) exti256_t;
typedef unsigned _ExtInt(384) exti384_t;
typedef unsigned _ExtInt(512) exti512_t;
typedef unsigned _ExtInt(768) exti768_t;

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

inline void emul256(exti512_t& z, const exti256_t& x, const exti256_t& y)
{
	z = exti512_t(x) * exti512_t(y);
}

inline void esqr256(exti512_t& y, const exti256_t& x)
{
	y = exti512_t(x) * exti512_t(x);
}

inline void emul384(exti768_t& z, const exti384_t& x, const exti384_t& y)
{
	z = exti768_t(x) * exti768_t(y);
}

inline void esqr384(exti768_t& y, const exti384_t& x)
{
	y = exti768_t(x) * exti768_t(x);
}
