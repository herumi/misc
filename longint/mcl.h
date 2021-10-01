#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int mcl_init();

typedef void (*void3u)(uint64_t *z, const uint64_t *x, const uint64_t *y);
// N = 11
// z[N*2] = x[N] * y[N]
extern void3u mcl_mulPre;

#ifdef __cplusplus
}
#endif
