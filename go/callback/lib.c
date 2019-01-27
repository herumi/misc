#include "lib.h"

static FuncType s_callbackC;

void setCallbackC(FuncType f)
{
	s_callbackC = f;
}

void callCallbackC(int x)
{
	printf("callCallbackC x=%d\n", x);
	int ret = s_callbackC(x + 1);
	printf("callCallbackC ret=%d\n", ret);
}

