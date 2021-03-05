#include "api.h"

extern "C" {

API int sub(int x, int y)
{
	return x - y;
}

API void setMem(char *p, int n)
{
	for (int i = 0; i < n; i++) p[i] = i;
}

}

