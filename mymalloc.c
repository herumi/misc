#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
//#define USE_READLINK
#ifdef USE_READLINK
#include <unistd.h>
#endif

FILE *mie_fp;
#define dprintf(...) fprintf(mie_fp, __VA_ARGS__)

typedef struct {
	char *p;
	size_t size;
	size_t n;
	int alloc;
} Info;

#define MAX_MALLOC_NUM 100000
#define MAX_MALLOC_SIZE (1024 * 1024 * 128)
const size_t margin = 8;
const char head[] = "\x11\x22\x33\x44\x55\x66\x77\x88";
const char tail[] = "\x99\xaa\xbb\xcc\xdd\xee\xff\x01";
__attribute__((aligned(32))) char mie_buffer[MAX_MALLOC_SIZE];
Info mie_tbl[MAX_MALLOC_NUM];
size_t mie_tblNum;
size_t mie_pos;
const void *mie_stopPtr;

void putInfo()
{
	fprintf(stderr, "putInfo mie_buffer=%p mie_tblNum=%zd mie_pos=%zd\n", mie_buffer, mie_tblNum, mie_pos);
#ifdef USE_READLINK
	char buf[1024];
	int ret = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
	if (ret < 0) {
		perror("readlink");
		return;
	}
	buf[ret] = '\0';
	fprintf(stderr, "self %s\n", buf);
#endif
}

static void putHex(const char *p, size_t n)
{
	fprintf(mie_fp, "putHex %p %zd\n", p, n);
	size_t i;
	for (i = 0; i < n; i++) {
		fprintf(mie_fp, "%02x ", (unsigned char)p[i]);
	}
	fprintf(mie_fp, "\n");
}

static void setStopPtr()
{
	const char *ptr = getenv("MIE_STOP_PTR");
	if (ptr == NULL) return;
	char *endp;
	mie_stopPtr = (const void*)strtol(ptr, &endp, 0);
	if (mie_stopPtr) {
		fprintf(stderr, "set MIE_STOP_PTR=%p\n", mie_stopPtr);
	}
}

__attribute__((constructor)) static void mie_init()
{
	const char *name = "./mymalloc.log";
	putInfo();
	mie_fp = fopen(name, "w");
	if (mie_fp == NULL) {
		fprintf(stderr, "can't open %s\n", name);
		mie_fp = stderr;
	}
	setStopPtr();
}
static void *my_alloc(size_t n, size_t size, size_t align)
{
	char *p;
	Info *pi;
	if (mie_tblNum == MAX_MALLOC_NUM) {
		fprintf(stderr, "QQQ mymalloc : no malloc num\n");
		exit(1);
	}
	mie_pos += margin;
	mie_pos = (mie_pos + align - 1) & ~(align - 1);
	if (mie_pos + size * n > MAX_MALLOC_SIZE) {
		fprintf(stderr, "QQQ mymalloc : no malloc n=%zd, mie_pos=%zd\n", n, size);
		exit(1);
	}
	p = mie_buffer + mie_pos;
	memcpy(p - margin, head, margin);
	pi = &mie_tbl[mie_tblNum];
	pi->p = p;
	pi->size = size;
	pi->n = n;
	pi->alloc = 1;
	mie_tblNum++;
	memcpy(p + size, tail, margin);
	mie_pos += size + margin;
	if (p == mie_stopPtr) {
		fprintf(stderr, "QQQ find stopPtr %p\n", p);
	}
	return p;
}
int mie_posix_memalign(void **memptr, size_t alignment, size_t size)
{
	void *p = my_alloc(1, size, alignment);
	if (p == NULL) return 1;
	*memptr = p;
	return 0;
}
void *mie_aligned_alloc(size_t alignment, size_t size)
{
	return my_alloc(1, size, alignment);
}

void mie_free(void *ptr)
{
	dprintf("free %p", ptr);
	char *p = (char*)ptr;
	int i;
	if (ptr == NULL) return;
	for (i = 0; i < mie_tblNum; i++) {
		Info *pi = &mie_tbl[i];
		if (pi->p == p) {
			if (pi->alloc == 0) {
				fprintf(stderr, " QQQ double free %p\n", ptr);
				exit(1);
			}
			pi->alloc = 0;
			dprintf("[%d]\n", i);
			if (memcmp(p - margin, head, margin) != 0) {
				dprintf("QQQ broken head:"); putHex(p - margin, margin);
				putHex(head, margin);
			}
			const char *q = p + pi->size * pi->n;
			if (memcmp(q, tail, margin) != 0) {
				dprintf("QQQ broken tail:"); putHex(q, margin);
				putHex(head, margin);
			}
			return;
		}
	}
	fprintf(stderr, " QQQ mie_free : not found %p\n", ptr);
	putInfo();
}

void *mie_malloc(size_t size)
{
	void *p = my_alloc(1, size, 16);
	dprintf("malloc[%zd] %zd %p\n", mie_tblNum - 1, size, p);
	return p;
}

void *mie_calloc(size_t n, size_t size)
{
	void *p = my_alloc(n, size, 16);
	memset(p, 0, n * size);
	dprintf("calloc[%zd] %zd %zd %p\n", mie_tblNum - 1, n, size, p);
	return p;
}
void *mie_realloc(void *ptr, size_t size)
{
	dprintf("realloc %p %zd : ", ptr, size);
	void *newPtr;
	if (size == 0) {
		mie_free(ptr);
		newPtr = ptr;
		goto exit;
	}
	newPtr = my_alloc(1, size, 16);
	if (ptr == NULL) goto exit;
	memmove(newPtr, ptr, size);
	mie_free(ptr);
exit:
	dprintf("realloc[%zd] end %p\n", mie_tblNum - 1, newPtr);
	return newPtr;
}
char *mie_strdup(const char *str)
{
	size_t len = strlen(str);
	dprintf("strdup %p %zd : ", str, len);
	char *newPtr = mie_malloc(len + 1);
	memcpy(newPtr, str, len + 1);
	dprintf("strdup[%zd] end %p\n", mie_tblNum - 1, newPtr);
	return newPtr;
}
void mie_dump(FILE *fp)
{
	fprintf(fp, "my_dstr num=%zd pos=%zd\n", mie_tblNum, mie_pos); putInfo();
	int i;
	int notFree = 0;
	for (i = 0; i < mie_tblNum; i++) {
		const Info *pi = &mie_tbl[i];
		if (pi->alloc) {
			fprintf(fp, "not free %p %zd %zd\n", pi->p, pi->size, pi->n);
			notFree++;
		}
	}
	fprintf(fp, "# notFree=%d\n", notFree);
}
void mie_dstr()
{
	mie_dump(stderr);
	if (mie_fp != stderr) {
		mie_dump(mie_fp);
	}
	putHex(mie_buffer, 200);
}
