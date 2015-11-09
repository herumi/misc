/*
	gcc -m32 -O0 printerr.c
	./a.out `printf "abc"`,%x,%x,%x,%x,%x,%x,
*/
#include <stdio.h>

int main(int argc, char *argv[])
{
	char msg[] = "abc";
	char buf[1024];
	printf("msg=%s\n", msg);
	printf("addr msg=%p, buf=%p\n", msg, buf);
	strcpy(buf, argv[1]);
	printf(buf);
	printf("msg=%s\n", msg);
}
