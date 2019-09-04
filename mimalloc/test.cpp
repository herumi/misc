/*
	g++ -O3 -DNDEBUG -I <mimalloc>/include <mimalloc>/build/libmimalloc.a -lpthread -lgmpxx -lgmp
*/
#include <gmp.h>
#include <mimalloc.h>
struct UseMiMalloc {
	static void* mi_malloc_wrapper(size_t n)
	{
//printf("malloc %zd\n", n);
		return mi_malloc(n);
	}
	static void* mi_realloc_wrapper(void *p, size_t, size_t n)
	{
//printf("realloc %p %zd\n", p, n);
		return mi_realloc(p, n);
	}
	static void mi_free_wrapper(void *p, size_t)
	{
//printf("free %p\n", p);
		mi_free(p);
	}
	UseMiMalloc()
	{
		puts("set GMP memory functions before using mpz_class");
		mp_set_memory_functions(mi_malloc, mi_realloc_wrapper, mi_free_wrapper);
//		mp_set_memory_functions(mi_malloc_wrapper, mi_realloc_wrapper, mi_free_wrapper);
	}
};

#include <gmpxx.h>
#include <time.h>

void putTime(const char *msg, clock_t begin, int n)
{
	printf("%s %.2f usec\n", msg, (clock() - begin) / double(CLOCKS_PER_SEC) / n * 1e6);
}

const int N = 1000000;

void* (*g_malloc)(size_t n);
void (*g_free)(void *p);

void mallocTime()
{
	clock_t begin = clock();
	for (int i = 0; i < N; i++) {
		char *p = (char *)malloc(32);
		free(p);
	}
	putTime("malloc/free", begin, N);
}

void mi_mallocTime()
{
	clock_t begin = clock();
	for (int i = 0; i < N; i++) {
		char *p = (char *)mi_malloc(32);
		mi_free(p);
	}
	putTime("mi_malloc/mi_free", begin, N);
}

void g_mallocTime(const char *msg)
{
	clock_t begin = clock();
	for (int i = 0; i < N; i++) {
		char *p = (char *)g_malloc(32);
		g_free(p);
	}
	putTime(msg, begin, N);
}

void mpzTime()
{
	clock_t begin = clock();
	for (int i = 0; i < N; i++) {
		mpz_class x = 3;
	}
	putTime("mpz_class", begin, N);
}

int main()
{
	mallocTime();
	mi_mallocTime();
	mpzTime();
	g_malloc = malloc;
	g_free = free;
	g_mallocTime("malloc ptr");
	g_malloc = mi_malloc;
	g_free = mi_free;
	g_mallocTime("mi_malloc ptr");
	UseMiMalloc use;
	mpzTime();
}

