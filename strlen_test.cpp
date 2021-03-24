#include <stdio.h>
#include <string.h>
#include <cybozu/benchmark.hpp>


size_t internal_strlen(const char *s) {
  size_t i = 0;
  while (s[i]) i++;
  return i;
}

size_t (*g_p1)(const char*) = strlen;
size_t (*g_p2)(const char*) = internal_strlen;

int main()
{
	const size_t N = 256;
	char buf[N];
	memset(buf, 1, N);
	buf[N - 1] = '\0';
	const int C = 10000;
	CYBOZU_BENCH_C("C       strlen", C, g_p1, buf);
	CYBOZU_BENCH_C("inernal_strlen", C, g_p2, buf);

	printf("len=%zd %zd\n", g_p1(buf), g_p2(buf));
}
