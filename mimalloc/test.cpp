/*
	g++ -O3 -DNDEBUG -I <mimalloc>/include <mimalloc>/build/libmimalloc.a -lpthread -lgmpxx -lgmp
*/
#include <gmp.h>
#include <mimalloc.h>
struct UseMiMalloc {
	static void* mi_realloc_wrapper(void *p, size_t, size_t n)
	{
		return mi_realloc(p, n);
	}
	static void mi_free_wrapper(void *p, size_t)
	{
		mi_free(p);
	}
	UseMiMalloc()
	{
		puts("set GMP memory functions before using mpz_class");
		mp_set_memory_functions(mi_malloc, mi_realloc_wrapper, mi_free_wrapper);
	}
};

#include <gmpxx.h>
#include <time.h>

void putTime(const char *msg, clock_t begin, int n)
{
	printf("%s %.2f usec\n", msg, (clock() - begin) / double(CLOCKS_PER_SEC) / n * 1e6);
}

void mallocTime()
{
	const int N = 100000000;
	clock_t begin = clock();
	for (int i = 0; i < N; i++) {
		char *p = (char *)malloc(32);
		free(p);
	}
	putTime("malloc/free", begin, N);
}

void mi_mallocTime()
{
	const int N = 100000000;
	clock_t begin = clock();
	for (int i = 0; i < N; i++) {
		char *p = (char *)mi_malloc(32);
		mi_free(p);
	}
	putTime("mi_malloc/mi_free", begin, N);
}

void mpzTime()
{
	const int N = 1000000;
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
	UseMiMalloc use;
	mpzTime();
}

