#include <stdlib.h>

int main()
{
	char *p = malloc(5);
	// buffer overrun
	p[5] = 'a';
	free(p);
}
