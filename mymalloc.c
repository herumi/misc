#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
//#define USE_READLINK
#ifdef USE_READLINK
#include <unistd.h>
#endif

FILE *mie_fp;
#define dprintf(...) if (mie_fp) fprintf(mie_fp, __VA_ARGS__)

typedef struct {
	char *p;
	size_t size;
	size_t n;
	int alloc;
} Info;

#define MAX_MALLOC_NUM 1000000
#define MAX_MALLOC_SIZE (1024 * 1024 * 1024)
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
	dprintf("putInfo mie_buffer=%p mie_tblNum=%zd mie_pos=%zd\n", mie_buffer, mie_tblNum, mie_pos);
#ifdef USE_READLINK
	char buf[1024];
	int ret = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
	if (ret < 0) {
		perror("readlink");
		return;
	}
	buf[ret] = '\0';
	dprintf("self %s\n", buf);
#endif
}

void mie_verify()
{
	dprintf("my_dstr num=%zd pos=%zd\n", mie_tblNum, mie_pos);
	putInfo();
	int i;
	int notFree = 0;
	for (i = 0; i < mie_tblNum; i++) {
		const Info *pi = &mie_tbl[i];
		if (pi->alloc) {
			dprintf("not free[%d] %p %zd %zd\n", i, pi->p, pi->size, pi->n);
			notFree++;
		}
	}
	dprintf("# notFree=%d\n", notFree);
}

static void putHex(const char *p, size_t n)
{
	dprintf("putHex %p %zd\n", p, n);
	size_t i;
	for (i = 0; i < n; i++) {
		dprintf("%02x ", (unsigned char)p[i]);
	}
	dprintf("\n");
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

//__attribute__((constructor))
void mie_init()
{
	static int init = 0;
	if (init) return;
	init = 1;
	const char *name = "/tmp/mymalloc.log";
	mie_fp = fopen(name, "w");
	if (mie_fp == NULL) {
		fprintf(stderr, "can't open %s\n", name);
		mie_fp = stderr;
	}
	putInfo();
	setStopPtr();
	atexit(mie_verify);
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
		dprintf("QQQ mymalloc : no malloc n=%zd, mie_pos=%zd\n", n, size);
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
		dprintf("QQQ find stopPtr %p\n", p);
	}
	return p;
}
int mie_posix_memalign(void **memptr, size_t align, size_t size)
{
	void *p = my_alloc(1, size, align);
	*memptr = p;
	dprintf("mie_posix_memalign[%zd] %zd %zd %p\n", mie_tblNum - 1, align, size, p);
	return (p == NULL);
}
void *mie_aligned_alloc(size_t align, size_t size)
{
	void *p = my_alloc(1, size, align);
	dprintf("mie_aligned_alloc[%zd] %zd %zd %p\n", mie_tblNum - 1, align, size, p);
	return p;
}
void *mie_memalign(size_t align, size_t size)
{
	void *p = my_alloc(1, size, align);
	dprintf("mie_memalign[%zd] %zd %zd %p\n", mie_tblNum - 1, align, size, p);
	return p;
}

void *mie_malloc(size_t size)
{
	void *p = my_alloc(1, size, 16);
	dprintf("malloc[%zd] %zd %p\n", mie_tblNum - 1, size, p);
	return p;
}

#define INFO_DOUBLE_FREE (-1)
#define INFO_BROKEN_HEAD (-2)
#define INFO_BROKEN_TAIL (-3)
#define INFO_NOT_FOUND (-4)
/*
	found if positive or zero
	err otherwise
*/
static int getInfoIdx(const void *p)
{
	int i;
	for (i = 0; i < mie_tblNum; i++) {
		Info *pi = &mie_tbl[i];
		if (pi->p == p) {
			if (pi->alloc == 0) {
				dprintf(" QQQ double free\n");
				return INFO_DOUBLE_FREE;
			}
			if (memcmp(p - margin, head, margin) != 0) {
				dprintf("QQQ broken head:");
				putHex(p - margin, margin);
				return INFO_BROKEN_HEAD;
			}
			const char *q = p + pi->size * pi->n;
			if (memcmp(q, tail, margin) != 0) {
				dprintf("QQQ broken tail:");
				putHex(q, margin);
				return INFO_BROKEN_TAIL;
			}
			return i;
		}
	}
	dprintf(" QQQ mie_free : not found\n");
	return INFO_NOT_FOUND;
}

void mie_free(void *p)
{
	if (p == NULL) return;
	dprintf("free %p\n", p);
	int idx = getInfoIdx(p);
	if (idx >= 0) {
		mie_tbl[idx].alloc = 0;
	}
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
size_t mie_malloc_usable_size(void *p)
{
	if (p == NULL) return 0;
	dprintf("malloc_usable_size %p ", p);
	int idx = getInfoIdx(p);
	if (idx >= 0) {
		size_t size = mie_tbl[idx].size * mie_tbl[idx].n;
		dprintf("%zd\n", size);
		return size;
	}
}

#ifdef MYMALLOC_PRELOAD
int posix_memalign(void **memptr, size_t align, size_t size)
{
	return mie_posix_memalign(memptr, align, size);
}
void *aligned_alloc(size_t align, size_t size)
{
	return mie_aligned_alloc(align, size);
}
void *memalign(size_t align, size_t size)
{
	return mie_memalign(align, size);
}
void *malloc(size_t size)
{
	return mie_malloc(size);
}
void free(void *ptr)
{
	mie_free(ptr);
}
void *calloc(size_t n, size_t size)
{
	return mie_calloc(n, size);
}
void *realloc(void *ptr, size_t size)
{
	return mie_realloc(ptr, size);
}
size_t malloc_usable_size(void *p)
{
	return mie_malloc_usable_size(p);
}
#endif

