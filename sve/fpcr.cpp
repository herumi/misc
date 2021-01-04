#include <cybozu/test.hpp>

#include "low_func.h"

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

CYBOZU_TEST_AUTO(el1)
{
	Type_id_aa64isa0_el1 type = xbyak_aarch64_get_id_aa64isar0_el1();
	printf("type:atomic=%d\n", type.atomic);
}

