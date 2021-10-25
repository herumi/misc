#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// n = 9 or 11
int mcl_init(int n);

typedef void (*void3u)(uint64_t *z, const uint64_t *x, const uint64_t *y);
// n = 9 or 11
// z[n*2] = x[n] * y[n]
extern void3u mcl_mulPre;
// z[n] = Montgomery(x[n], y[n]);
extern void3u mcl_mont;

#ifdef __cplusplus
}
#endif
