#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#define __USE_GNU
#include <dlfcn.h>

FILE *mie_fp;
#define dprintf(...) if (mie_fp) fprintf(mie_fp, __VA_ARGS__)

#define MAX_MALLOC_SIZE (1024 * 1024 * 1024)
#define MAX_MALLOC_NUM 1000000
#define HASH_NUM 100003
#define LIST_NUM (MAX_MALLOC_NUM / HASH_NUM)
//#define MAX_STACK_NUM 4

typedef struct {
	char *p;
	size_t size;
#ifdef MAX_STACK_NUM
	void *stack[MAX_STACK_NUM];
#endif
	int alloc;
} Info;

const size_t margin = 8;
const char head[] = "\x11\x22\x33\x44\x55\x66\x77\x88";
const char tail[] = "\x99\xaa\xbb\xcc\xdd\xee\xff\x01";
__attribute__((aligned(4096))) char mie_buffer[MAX_MALLOC_SIZE];
size_t mie_pos;
const void *mie_stopPtr;
Info mie_tbl[HASH_NUM][LIST_NUM];
int mie_tblNum[HASH_NUM];
int mie_allocNum;
int getStackTrace = 0;

static int getHash(const void *p)
{
	return ((size_t)p / 8) % HASH_NUM;
}

static void putInfo(const Info *pi)
{
	dprintf("%p %zd", pi->p, pi->size);
#ifdef MAX_STACK_NUM
	if (getStackTrace) {
		int i;
		for (i = 0; i < MAX_STACK_NUM; i++) {
			const void *p = pi->stack[i];
			if (p) dprintf(" %p", p);
		}
	}
#endif
	dprintf("\n");
}
void mie_verify()
{
	dprintf("my_dstr mie_buffer=%p mie_pos=%zd mie_allocNum=%d \n", mie_buffer, mie_pos, mie_allocNum);
	int i, j;
	for (i = 0; i < HASH_NUM; i++) {
		const Info *tbl = &mie_tbl[i][0];
		int n = mie_tblNum[i];
		for (j = 0; j < n; j++) {
			const Info *pi = &tbl[j];
			if (pi->alloc) {
				dprintf("not free ");
				putInfo(pi);
			}
		}
	}
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

#ifdef MYMALLOC_PRELOAD
__attribute__((constructor))
#endif
void mie_init()
{
	static int init = 0;
	if (init) return;
	init = 1;
#if 1
	mie_fp = stderr;
#else
	const char *name = "/tmp/mymalloc.log";
	mie_fp = fopen(name, "w");
	if (mie_fp == NULL) {
		fprintf(stderr, "can't open %s\n", name);
		mie_fp = stderr;
	}
#endif
	dprintf("mie_buffer=%p mie_pos=%zd mie_allocNum=%d \n", mie_buffer, mie_pos, mie_allocNum);
	setStopPtr();
	atexit(mie_verify);
//	getStackTrace = 1;
}
static void *my_alloc(size_t size, size_t align)
{
#if 0
	if (mie_fp == NULL) {
		static void *(*org_malloc)(size_t) = NULL;
		if (org_malloc == NULL) {
			org_malloc = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
			if (org_malloc == NULL) {
				perror("dlsym");
				exit(1);
			}
		}
		return org_malloc(size);
	}
#endif
	char *p;
	Info *pi;
	mie_pos += margin;
	mie_pos = (mie_pos + align - 1) & ~(align - 1);
	if (mie_pos + size > MAX_MALLOC_SIZE) {
		dprintf("QQQ mymalloc : no malloc size=%zd\n", size);
		exit(1);
	}
	p = mie_buffer + mie_pos;
	memcpy(p - margin, head, margin);
	int idx = getHash(p);
	int pos = mie_tblNum[idx]++;
	if (pos == LIST_NUM) {
		dprintf("QQQ mymalloc : no malloc idx=%d\n", idx);
		exit(1);
	}
	pi = &mie_tbl[idx][pos];
	pi->p = p;
	pi->size = size;
	pi->alloc = 1;
	memcpy(p + size, tail, margin);
	mie_pos += size + margin;
	mie_allocNum++;
#ifdef MAX_STACK_NUM
	if (getStackTrace) {
		int n = backtrace(pi->stack, MAX_STACK_NUM);
		int i;
		for (i = n; i < MAX_STACK_NUM; i++) pi->stack[i] = NULL;
	}
#endif
	if (p == mie_stopPtr) {
		dprintf("QQQ find stopPtr %p\n", p);
	}
//	dprintf("%d:%d [%d] %zd %zd %p\n", idx, pos, mie_allocNum, size, align, p);
	dprintf("%zd %zd %p\n", size, align, p);
	return p;
}

int mie_posix_memalign(void **memptr, size_t align, size_t size)
{
	dprintf("mie_posix_memalign ");
	void *p = my_alloc(size, align);
	*memptr = p;
	return (p == NULL);
}
void *mie_aligned_alloc(size_t align, size_t size)
{
	dprintf("mie_aligned_alloc ");
	void *p = my_alloc(size, align);
	return p;
}
void *mie_memalign(size_t align, size_t size)
{
	dprintf("mie_memalign ");
	void *p = my_alloc(size, align);
	return p;
}

void *mie_malloc(size_t size)
{
	dprintf("mie_malloc ");
	void *p = my_alloc(size, 16);
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
static int getInfoIdx(int *pIdx, void *p)
{
	int i;
	int idx = getHash(p);
	*pIdx = idx;
	Info *tbl = &mie_tbl[idx][0];
	int n = mie_tblNum[idx];
	for (i = 0; i < n; i++) {
		Info *pi = &tbl[i];
		if (pi->p == p) {
			if (pi->alloc == 0) {
				dprintf(" QQQ double free ");
				putInfo(pi);
				return INFO_DOUBLE_FREE;
			}
			if (memcmp(p - margin, head, margin) != 0) {
				dprintf("QQQ broken head:");
				putInfo(pi);
				putHex(p - margin, margin);
				return i;
//				return INFO_BROKEN_HEAD;
			}
			const char *q = p + pi->size;
			if (memcmp(q, tail, margin) != 0) {
				dprintf("QQQ broken tail:");
				putInfo(pi);
				putHex(q, margin);
//				return INFO_BROKEN_TAIL;
				return i;
			}
			return i;
		}
	}
	dprintf("QQQ not found\n");
	return INFO_NOT_FOUND;
}

void mie_free(void *p)
{
	if (p == NULL) return;
	dprintf("free %p ", p);
	int idx;
	int pos = getInfoIdx(&idx, p);
	if (pos >= 0) {
		mie_tbl[idx][pos].alloc = 0;
		mie_allocNum--;
		dprintf("\n");
	}
#if 0
	static void (*org_free)(void*) = NULL;
	if (org_free == NULL) {
		org_free = (void (*)(void*))dlsym(RTLD_NEXT, "free");
		if (org_free == NULL) {
			perror("dlsym free");
			exit(1);
		}
	}
	org_free(p);
#endif
}

void *mie_calloc(size_t n, size_t size)
{
	dprintf("mie_calloc ");
	void *p = my_alloc(n * size, 16);
	memset(p, 0, n * size);
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
	newPtr = my_alloc(size, 16);
	if (ptr == NULL) goto exit;
	memmove(newPtr, ptr, size);
	mie_free(ptr);
exit:
	dprintf("realloc end %p\n", newPtr);
	return newPtr;
}
char *mie_strdup(const char *str)
{
	size_t len = strlen(str);
	dprintf("mie_strdup %p %zd ", str, len);
	char *newPtr = my_alloc(len + 1, 16);
	memcpy(newPtr, str, len + 1);
	return newPtr;
}
size_t mie_malloc_usable_size(void *p)
{
	if (p == NULL) return 0;
	dprintf("malloc_usable_size %p ", p);
	int idx;
	int pos = getInfoIdx(&idx, p);
	if (pos >= 0) {
		size_t size = mie_tbl[idx][pos].size;
		dprintf("[%d:%d] %zd\n", pos, idx, size);
		return size;
	}
#if 1
	return 0;
#else
	static size_t (*org_malloc_usable_size)(void*) = NULL;
	if (org_malloc_usable_size == NULL) {
		org_malloc_usable_size = (size_t (*)(void*))dlsym(RTLD_NEXT, "malloc_usable_size");
		if (org_malloc_usable_size == NULL) {
			perror("dlsym malloc_usable_size");
			exit(1);
		}
	}
	return org_malloc_usable_size(p);
#endif
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

