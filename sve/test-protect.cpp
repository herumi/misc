#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

enum ProtectMode {
	PROTECT_RW = 0, // read/write
	PROTECT_RWE = 1, // read/write/exec
	PROTECT_RE = 2 // read/exec
};
static inline bool protect(const void *addr, size_t size, int protectMode) {
    const int c_rw = PROT_READ | PROT_WRITE;
    const int c_rwe = PROT_READ | PROT_WRITE | PROT_EXEC;
    const int c_re = PROT_READ | PROT_EXEC;
    int mode;
    switch (protectMode) {
      case PROTECT_RW:
        mode = c_rw;
        break;
      case PROTECT_RWE:
        mode = c_rwe;
        break;
      case PROTECT_RE:
        mode = c_re;
        break;
      default:
        return false;
    }
    size_t pageSize = sysconf(_SC_PAGESIZE);
    printf("pageSize=%zd\n", pageSize);
    size_t iaddr = reinterpret_cast<size_t>(addr);
    size_t roundAddr = iaddr & ~(pageSize - static_cast<size_t>(1));
    int ret =  mprotect(reinterpret_cast<void *>(roundAddr), size + (iaddr - roundAddr), mode);
	printf("mprotect ret=%d\n", ret);
	return ret == 0;
}

int main()
{
	const size_t N = 4096;
	alignas(N) static char buf[N];
	printf("buf=%p\n", buf);
	protect(buf, N, PROTECT_RWE);
	void *p;
	int ret = posix_memalign(&p, N, N);
	printf("posix_memalign ret=%d\n", ret);
	printf("p=%p\n", p);
	protect(p, N, PROTECT_RWE);
}
