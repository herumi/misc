#include <stdio.h>
#include <cybozu/mmap.hpp>
#include <cybozu/atoi.hpp>
#include "../../../haskell/integer.hpp"

int main()
{
	cybozu::Mmap m("pi.txt");
	const char *p = m.get();
	for (int i = 0; i < m.size() - 10; i++) {
		uint64_t v = cybozu::atoi(p + i, 10);
		if (isMaybePrime(v)) {
			printf("found %lld\n", (long long)v);
			break;
		}
	}
}
