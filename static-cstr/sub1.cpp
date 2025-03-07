#include <stdio.h>
#include "common.h"

namespace {

static struct X1 {
	X1() { puts("sub1 X1 cstr"); }
} x1;

static void CSTR initSub1()
{
	puts("initSub1");
}

#ifdef _MSC_VER
#pragma section(".CRT$XCT", read)
__declspec(allocate(".CRT$XCT")) void(*ptr_initSub1)() = initSub1;
#endif

static struct X2 {
	X2() { puts("sub1 X2 cstr"); }
} x2;

}
