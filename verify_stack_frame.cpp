/*
g++ -m32 -g calling.cpp -Wall -finstrument-functions && ./a.out
*/
#include <stdio.h>
#include <algorithm>
#include <stdint.h>
#include <execinfo.h>
#include "verify_stack_frame.hpp"

static const int MAX_FROM_POS = 256;
static const int STACK_BUF = 6;
static void *fromTbl[MAX_FROM_POS + 1];
int fromPos = 0;

void verifyStackFrame()
{
	printf("verfy\n");
	void *buf[STACK_BUF];
	int size = backtrace(buf, STACK_BUF);
#if 1
	for (int i = 0; i < size; i++) {
		printf("buf[%d]=%p\n", i, buf[i]);
	}
	for (int i = 0; i < fromPos; i++) {
		printf("fromTbl[%d]=%p\n", i, fromTbl[i]);
	}
#endif
#if 1
#else
	for (int i = 2; i < size; i++) {
		const void *p = buf[i];
		int pos = fromPos - 1 - i;
		if (pos < 0) return;
		const void *q = fromTbl[pos];
		if (p != q) {
			printf("err [%d] %p %p\n", i, p, q);
		}
	}
#endif
}

extern "C" {
void __cyg_profile_func_enter(void *funcAddr, void *callSite)
__attribute__((no_instrument_function));
void __cyg_profile_func_exit(void *funcAddr, void *callSite)
__attribute__((no_instrument_function));
}

#define UNUSED __attribute__((unused))
void __cyg_profile_func_enter(UNUSED void* funcAddr, UNUSED void* callSite)
{
	printf("enter=%p, from %p [%d]\n", funcAddr, callSite, fromPos);
	if (fromPos == MAX_FROM_POS) {
		printf("fromPos is max\n");
		exit(1);
	}
	fromTbl[fromPos++] = callSite;
	verifyStackFrame();
}

void __cyg_profile_func_exit(UNUSED void *funcAddr, UNUSED void *callSite)
{
//	printf("leave=%p, from %p\n", funcAddr, callSite);
	fromPos--;
}

