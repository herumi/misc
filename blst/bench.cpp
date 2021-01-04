#include "blst.hpp"
#include <cybozu/benchmark.hpp>

void put(const char *msg, const void *buf, size_t n)
{
	const uint8_t *src = (const uint8_t *)buf;
	if (msg && *msg) printf("%s\n", msg);
	for (size_t i = 0; i < n; i++) {
		printf("%02x", src[i]);
		if ((i % 48) == 47) printf("\n");
	}
}

void putRev(const void *buf, size_t n)
{
	const uint8_t *src = (const uint8_t *)buf;
	for (size_t i = 0; i < n; i++) {
		printf("%02x", src[n - 1 - i]);
	}
	printf("\n");
}

void put(const blst::blst_fp* x)
{
	uint32_t a[12];
	blst_uint32_from_fp(a, x);
	putRev(a, sizeof(a));
}

void put(const char *msg, const blst::PT& e)
{
	printf("%s\n", msg);
	for (int i = 0; i < 12; i++) {
		put(&((const blst::blst_fp12*)(&e))->fp6[0].fp2[0].fp[i]);
	}
}

void pairing(const blst::P1_Affine& P, const blst::P2_Affine& Q)
{
	blst::PT e(Q, P);
	e.final_exp();
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
	put("e", e);
	CYBOZU_BENCH_C("pairing", 1000, pairing, P, Q);
	CYBOZU_BENCH_C("ML", 1000, PT, Q, P);
	CYBOZU_BENCH_C("FE", 1000, e.final_exp);
}
