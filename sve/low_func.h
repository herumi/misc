#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t xbyak_aarch64_get_fpcr(void);
void xbyak_aarch64_set_fpcr(uint32_t x);

typedef struct {
  int resv0:4;
  int aes:4;
  int sha1:4;
  int sha2:4;
  int crc32:4;
  int atomic:4;
  int resv1:4;
  int rdm:4;
  int resv2:12;
  int dp:4;
  int resv3:16;
} Type_id_aa64isa0_el1;

Type_id_aa64isa0_el1 xbyak_aarch64_get_id_aa64isar0_el1(void);


#ifdef __cplusplus
}
#endif
