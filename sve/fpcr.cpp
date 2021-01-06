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
#ifdef __clang__
	for (int i = 0; i < 4; i++) {
		unsigned int rounding = i << 22;
		set_fpcr(rounding);
		CYBOZU_TEST_EQUAL(get_fpcr(), rounding);
	}
#else
	const unsigned int a = __builtin_aarch64_get_fpcr();
	const unsigned int b = get_fpcr();
	CYBOZU_TEST_EQUAL(a, b);
	for (int i = 0; i < 4; i++) {
		unsigned int rounding = i << 22;
		set_fpcr(rounding);
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
	Type_id_aa64isar0_el1 type = get_id_aa64isar0_el1();
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
	Type_id_aa64pfr0_el1 type = get_id_aa64pfr0_el1();
	printf("el0=%d\n", type.el0);
	printf("el1=%d\n", type.el1);
	printf("el2=%d\n", type.el2);
	printf("el3=%d\n", type.el3);
	printf("fp=%d\n", type.fp);
	printf("advsimd=%d\n", type.advsimd);
	printf("gic=%d\n", type.gic);
	printf("ras=%d\n", type.ras);
	printf("sve=%d\n", type.sve);
#endif
}

//#define USE_LOW_FUNC
#ifdef USE_LOW_FUNC
extern "C"
#endif
uint64_t get_zcr_el1()
#ifdef USE_LOW_FUNC
;
#else
{
	uint64_t x;
//	asm __volatile__("mrs %[x], zcr_el1":[x]"=r"(x));
	asm __volatile__(".inst 0xd5381200":"=r"(x));
	return x;
}
#endif

#define Op0_shift	19
#define Op0_mask	0x3
#define Op1_shift	16
#define Op1_mask	0x7
#define CRn_shift	12
#define CRn_mask	0xf
#define CRm_shift	8
#define CRm_mask	0xf
#define Op2_shift	5
#define Op2_mask	0x7

#define sys_reg(op0, op1, crn, crm, op2) \
	(((op0) << Op0_shift) | ((op1) << Op1_shift) | \
	 ((crn) << CRn_shift) | ((crm) << CRm_shift) | \
	 ((op2) << Op2_shift))
#define SYS_ZCR_EL1			sys_reg(3, 0, 1, 2, 0)
/// ZCR_EL1=00181200

CYBOZU_TEST_AUTO(zcr_el1)
{
	printf("ZCR_EL1=%08x\n", SYS_ZCR_EL1);
	uint32_t mrs = 0xd5200000;
	// mrs =0xd5381200
	printf("mrs =%08x\n", SYS_ZCR_EL1|mrs);
	printf("zcr_el1=%08x\n", (int)get_zcr_el1());
	puts("ok");
}
