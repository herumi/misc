#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

inline uint32_t get_fpcr(void)
{
	uint64_t x;
	asm __volatile__("mrs %[x], fpcr":[x]"=r"(x));
	return x;
}
inline void set_fpcr(uint32_t x)
{
	uint64_t xx = x;
	asm __volatile__("msr fpcr, %[x]"::[x]"r"(xx));
}

struct Type_id_aa64isar0_el1 {
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
};

Type_id_aa64isar0_el1 get_id_aa64isar0_el1(void)
{
  Type_id_aa64isar0_el1 x;
  asm __volatile__("mrs %[x], id_aa64isar0_el1":[x]"=r"(x));
  return x;
}

struct Type_id_aa64pfr0_el1 {
  int el0:4;
  int el1:4;
  int el2:4;
  int el3:4;
  int fp:4;
  int advsimd:4;
  int gic:4;
  int ras:4;
  int sve:4;
  int resv0:28;
};

Type_id_aa64pfr0_el1 get_id_aa64pfr0_el1(void)
{
  Type_id_aa64pfr0_el1 x;
  asm __volatile__("mrs %[x], id_aa64pfr0_el1":[x]"=r"(x));
  return x;
}

#ifdef __cplusplus
}
#endif
