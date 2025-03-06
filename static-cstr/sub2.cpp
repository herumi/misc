#include <stdio.h>

#ifdef __GNUC__
	#define MCL_ATTRIBUTE __attribute__((constructor))
#else
	#define MCL_ATTRIBUTE
#endif

static void MCL_ATTRIBUTE initSub2()
{
	puts("initSub2");
}

#ifdef _MSC_VER
#pragma section(".CRT$XCU", read)
__declspec(allocate(".CRT$XCU")) void(*ptr_initSub2)() = initSub2;
#endif

