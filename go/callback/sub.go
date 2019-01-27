package main
/*
#include "lib.h"
int wrapCallbackGo(int); // exported from main.go
int wrapCallbackCgo(int x)
{
	printf("  wrapCallbackCgo x=%d\n", x);
	int ret = wrapCallbackGo(x + 1);
	printf("  wrapCallbackCgo ret=%d\n", ret);
	return ret;
}
*/
import "C"

