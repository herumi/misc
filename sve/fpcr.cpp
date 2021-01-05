#include <cybozu/test.hpp>

#include "low_func.h"
#include <stdint.h>

#if 0
uint64_t get_fpcr()
{
	uint64_t x;
	asm __volatile__("mrs %[x], fpcr":[x]"=r"(x));
	return x;
}

void set_fpcr(uint64_t x)
{
	asm __volatile__("msr fpcr, %[x]"::[x]"r"(x));
}
#endif

CYBOZU_TEST_AUTO(fpcr)
{
#ifdef __APPLE__
	for (int i = 0; i < 4; i++) {
		unsigned int rounding = i << 22;
		xbyak_aarch64_set_fpcr(rounding);
		CYBOZU_TEST_EQUAL(xbyak_aarch64_get_fpcr(), rounding);
	}
#else
	const unsigned int a = __builtin_aarch64_get_fpcr();
	const unsigned int b = xbyak_aarch64_get_fpcr();
	CYBOZU_TEST_EQUAL(a, b);
	for (int i = 0; i < 4; i++) {
		unsigned int rounding = i << 22;
		xbyak_aarch64_set_fpcr(rounding);
		CYBOZU_TEST_EQUAL(__builtin_aarch64_get_fpcr(), rounding);
	}
	__builtin_aarch64_set_fpcr(a);
#endif
}

CYBOZU_TEST_AUTO(id_aa64isar0_el1)
{
#ifdef __APPLE__
	printf("no id_aa64isar0_el1\n");
#else
	Type_id_aa64isar0_el1 type = xbyak_aarch64_get_id_aa64isar0_el1();
	printf("aes=%d\n", type.aes);
	printf("sha1=%d\n", type.sha1);
	printf("sha2=%d\n", type.sha2);
	printf("crc32=%d\n", type.crc32);
	printf("atomic=%d\n", type.atomic);
	printf("rdm=%d\n", type.rdm);
	printf("dp=%d\n", type.dp);
#endif
}

CYBOZU_TEST_AUTO(aa64pfr0_el1)
{
#ifdef __APPLE__
	printf("no aa64pfr0_el1\n");
#else
	Type_id_aa64pfr0_el1 type = xbyak_aarch64_get_id_aa64pfr0_el1();
	printf("el0=%d\n", type.el0);
	printf("el1=%d\n", type.el1);
	printf("el2=%d\n", type.el2);
	printf("el3=%d\n", type.el3);
	printf("fp=%d\n", type.fp);
	printf("advsimd=%d\n", type.advsimd);
	printf("gic=%d\n", type.gic);
	printf("ras=%d\n", type.ras);
#endif
}
