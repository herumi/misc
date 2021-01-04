#pragma once

#ifdef __cplusplus
extern "C" {
#endif

unsigned int xbyak_aarch64_get_fpcr(void);
void xbyak_aarch64_set_fpcr(unsigned int x);

#ifdef __cplusplus
}
#endif
