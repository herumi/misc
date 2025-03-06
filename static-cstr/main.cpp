#include <stdio.h>

#ifdef __GNUC__
	#define MCL_ATTRIBUTE __attribute__((constructor))
#else
	#define MCL_ATTRIBUTE
#endif

namespace {

static struct X1 {
	X1() { puts("X1 cstr"); }
} x1;

static void MCL_ATTRIBUTE initMain()
{
	puts("initMain");
}

#ifdef _MSC_VER
#pragma section(".CRT$XCU", read)
__declspec(allocate(".CRT$XCU")) void(*ptr_initMain)() = initMain;
#endif

static struct X2 {
	X2() { puts("X2 cstr"); }
} x2;

}

int main()
{
	puts("main");
	puts("");
}

namespace {

static struct X3 {
	X3() { puts("X3 cstr"); }
} x3;

}
