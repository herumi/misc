#pragma once
#include <stdio.h>
typedef int (*FuncType)(int);
void setCallbackC(FuncType f);
void callCallbackC(int x);
int wrapCallbackCgo(int x);

