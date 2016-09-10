#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define XBYAK_NO_OP_NAMES
#include "xbyak/xbyak.h"

const size_t codeSize = 256;

struct Mem {
	int patoi; // -20
	int pprintf; // -16
	char str[12]; // -12
	char code[codeSize];
} mem = {
	(int)atoi,
	(int)printf,
	"%d! = %u\n",
	"",
};

#define GET_OFFSET(x) ((int)&(mem.code) - (int)&(mem.x))

struct Code : public Xbyak::CodeGenerator {
	Code()
		: Xbyak::CodeGenerator(codeSize, mem.code)
	{
		Xbyak::CodeArray::protect(mem.code, codeSize, true);
		push(esi);
		push(edi);
		push(ebx);
		push(ebp);
		const size_t P = 4 * 4;

		call("@f");
	L("@@");
		pop(esi);
		const size_t adj = getSize() - 1;
		mov(ebp, ptr [esp + P + 4]); // argc
		cmp(ebp, 1);
		je(".skip");
		mov(ebp, ptr [esp + P + 8]); // argv
		mov(ebp, ptr [ebp + 4]); // argv[1]
		push(ebp);
		mov(ebp, ptr [esi - adj - GET_OFFSET(patoi)]);
		call(ebp);
		add(esp, 4);
		mov(ebp, eax);
	L(".skip");
		// ebp = n
		sub(esp, 4 * 3);
		lea(eax, ptr [esi - adj - GET_OFFSET(str)]);
		mov(ptr [esp], eax);
		mov(esi, ptr [esi - adj - GET_OFFSET(pprintf)]);

		xor_(ebx, ebx);
		inc(ebx); // i = 1
		mov(edi, ebx); // fac

	L(".lp");
		imul(edi, ebx);
		mov(ptr [esp + 4], ebx);
		mov(ptr [esp + 8], edi);
		call(esi);
		inc(ebx);
		cmp(ebx, ebp);
		jbe(".lp");

		add(esp, 4 * 3);

		pop(ebp);
		pop(ebx);
		pop(edi);
		pop(esi);
		ret();
	}
};

int main(int argc, char *argv[])
{
	Code code;
	if (argc == 2 && strcmp(argv[1], "-print") == 0) {
		// display code
		puts(
			"/*\n"
			"	cl fac.c on Windows\n"
			"	gcc fac.c -m32 -z execstack on Linux\n"
			"*/"
		);
		
		puts("#include <stdio.h>");
		puts("#include <stdlib.h>");
		puts("a = atoi;");
		puts("b = printf;");
		char c = 'c';
		size_t remain = sizeof(mem.str) + code.getSize();
		const unsigned char *p = (unsigned char*)&mem.str;
		do {
			if (p == code.getCode()) {
				printf("main = ");
			} else {
				printf("%c = ", c++);
			}
			union ic {
				unsigned int i;
				unsigned char c[4];
			} ic;
			size_t n = std::min(remain, 4U);
			for (size_t i = 0; i < n; i++) {
				ic.c[i] = *p++;
			}
			printf("0x%08x;\n", ic.i);
			remain -= n;
		} while (remain > 0);
	} else {
		((void(*)(int,char*[]))code.getCode())(argc, argv);
	}
}
