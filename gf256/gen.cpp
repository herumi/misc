#include <stdio.h>
#include "mul.h"

void gen_inv()
{
	printf("static const uint8_t invTbl[] = {\n");
	printf("0x00, ");
	for (int i = 1; i < 256; i++) {
		for (int j = 1; j < 256; j++) {
			if (mul(i, j) == 1) {
				printf("0x%02x, ", j);
				break;
			}
		}
		if ((i % 16) == 15) putchar('\n');
	}
	puts("};");
}

void gen_exp_log()
{
	uint8_t logTbl[256] = {};
	uint8_t g = 3; // generator of K-{0}
	uint8_t x = 1;
	puts("static const uint8_t expTbl[] = {");
	for (int i = 0; i < 255; i++) {
		printf("0x%02x, ", x); // expTbl[i] = g^i
		logTbl[x] = i; // logTbl[expTbl[i]] = i
		x = mul(x, g);
		if ((i % 16) == 15) putchar('\n');
	}
	puts("};");
	puts("static const uint8_t logTbl[] = {");
	for (int i = 1; i < 256; i++) {
		printf("0x%02x, ", logTbl[i]);
		if ((i % 16) == 15) putchar('\n');
	}
	puts("};");
}

int main()
{
	gen_inv();
	gen_exp_log();
}
