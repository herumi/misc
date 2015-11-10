/*
	gcc -m32 -O0 printerr.c
	echo `printf "abc"`,%x,%x,%x,%x,%x,%x, | ./a.out
*/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int f;
	char buf[0x400];
	printf("f=%x, &f=%p\n", f, &f);
    fgets(buf, sizeof(buf), stdin);
    printf("Hi, ");
    printf(buf);
    putchar('\n');
    f = 1;
    while (f != 0) {
        puts("Do you want the flag?");
        if (fgets(buf, 0x400, stdin) == 0) {
            return 0;
        }
        if (strcmp(buf, "no\n") == 0) {
            puts("I see. Good bye.");
            return 0;
        }
    }
    FILE *fp;
    fp = fopen("flag.txt", "r");
    fgets(buf, 0x400, fp);
    printf(buf);
    return 0;
}
