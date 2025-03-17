#include <stdio.h>

#ifndef PRIORITY
	#define PRIORITY 101
#endif

#ifdef __GNUC__
	#define CSTR __attribute__((constructor(PRIORITY)))
	#define INIT_PRIORITY(x) __attribute__((init_priority(x)))
#else
	#define CSTR
	#define INIT_PRIORITY(x)
#endif

namespace {

static struct X1 {
	X1() { puts("X1 cstr"); }
} x1 INIT_PRIORITY(1030);

static void CSTR initMain()
{
	puts("initMain");
}

#ifdef _MSC_VER
#pragma warning(default:5247)
#pragma warning(default:5248)
#pragma section(".CRT$XCT", read)
__declspec(allocate(".CRT$XCT")) void(*ptr_initMain)() = initMain;
#endif

static struct X2 {
	X2() { puts("X2 cstr"); }
} x2 INIT_PRIORITY(1010);

}

int main()
{
	puts("main");
	puts("");
}

namespace {

static struct X3 {
	X3() { puts("X3 cstr"); }
} x3 INIT_PRIORITY(1020);

}
