#define XBYAK_NO_OP_NAMES
#include "xbyak/xbyak.h"
#include <string.h>

struct Code : Xbyak::CodeGenerator {
	Code(void *p, int x)
		: Xbyak::CodeGenerator(4096, p)
	{
		mov(eax, x);
		ret();
	}
};

int main(int argc, char *argv[])
{
	argc--, argv++;
	bool callProtect = false;
	bool useMmap = false;
	if (argc == 1) {
		if (strcmp(*argv, "mmap") == 0) {
			useMmap = true;
		} else {
			callProtect = true;
		}
	}
	const int N = 70000;
	const int size = 4096;
	printf("callProtect %d\n", callProtect);
	printf("useMmap %d\n", useMmap);
	for (int i = 0; i < N; i++) {
		void *p;
		if (useMmap) {
			p = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			if (p == 0) {
				perror("mmap");
				printf("%d\n", i);
				return 1;
			}
		} else {
			p = Xbyak::AlignedMalloc(size, 4096);
		}
		if (!Xbyak::CodeArray::protect(p, size, true)) {
			printf("ERR %d\n", i);
			return 1;
		}
		Code c(p, i);
		printf("%d %p %d\n", i, p, c.getCode<int (*)()>()());
		if (callProtect) Xbyak::CodeArray::protect(p, size, false);
//		free(p);
	}
	printf("ok call %d\n", N);
}
