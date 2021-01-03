#include "blst.hpp"
#include <cybozu/benchmark.hpp>

void put(const char *msg, const void *buf, size_t n)
{
	const uint8_t *src = (const uint8_t *)buf;
	printf("%s\n", msg);
	for (size_t i = 0; i < n; i++) {
		printf("%02x", src[i]);
		if ((i % 48) == 47) printf("\n");
	}
	printf("\n");
}

int main()
{
	using namespace blst;
	P1_Affine P = P1_Affine::generator();
	P2_Affine Q = P2_Affine::generator();
	byte buf1[96];
	byte buf2[192];
	P.serialize(buf1);
	Q.serialize(buf2);
	put("P", buf1, sizeof(buf1));
	put("Q", buf2, sizeof(buf2));
	
	PT e(Q, P);
	e.final_exp();
	printf("sizeof(PT)=%zd\n", sizeof(PT));
	put("e", &e, sizeof(PT));
}
