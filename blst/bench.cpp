#include "blst.hpp"
#include <cybozu/benchmark.hpp>

#define MCL_USE_VINT
#define CYBOZU_DONT_USE_OPENSSL
#define MCL_MAX_BIT_SIZE 384
#define MCL_STATIC_CODE
#define PUT(x) std::cout << #x "=" << x << std::endl;
cybozu::CpuClock clk;
#include <mcl/bls12_381.hpp>
#include <cybozu/xorshift.hpp>

mcl::bn::GT g_blst_e;

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

const blst::blst_fp2* get_fp2(const blst::PT& e)
{
	return &((const blst::blst_fp12*)(&e))->fp6[0].fp2[0];
}

const blst::blst_fp* get_fp(const blst::PT& e)
{
	return &get_fp2(e)->fp[0];
}

void put(const char *msg, const blst::PT& e)
{
	printf("%s\n", msg);
	for (int i = 0; i < 12; i++) {
		put(&get_fp(e)[i]);
	}
}

void pairing(const blst::P1_Affine& P, const blst::P2_Affine& Q)
{
	blst::PT e(Q, P);
	e.final_exp();
}

void blst_bench()
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
	g_blst_e = *(mcl::bn::GT*)&e;
//	put("e", e);
	puts("--- blst ---");
	CYBOZU_BENCH_C("pairing", 1000, pairing, P, Q);
	CYBOZU_BENCH_C("ML", 1000, PT, Q, P);
	CYBOZU_BENCH_C("FE", 1000, e.final_exp);
	{
		blst_fp a = get_fp(e)[0];
		blst_fp b = get_fp(e)[1];
		CYBOZU_BENCH_C("fp::mul", 10000, blst_fp_mul, &a, &a, &b);
	}
	{
		blst_fp2 a = get_fp2(e)[0];
		blst_fp2 b = get_fp2(e)[1];
		CYBOZU_BENCH_C("fp2::mul", 10000, blst_fp2_mul, &a, &a, &b);
		CYBOZU_BENCH_C("fp2::sqr", 10000, blst_fp2_sqr, &a, &a);
	}
}

void mcl_bench_sub()
{
	using namespace mcl::bn;
	G1 P;
	G2 Q;
	P.setStr(
	"1 3685416753713387016781088315183077757961620795782546409894578378688607592378376318836054947676345821548104185464507"
	" 1339506544944476473020471379941921221584933875938349620426543736416511423956333506472724655353366534992391756441569",
	10);
	Q.setStr(
	"1 352701069587466618187139116011060144890029952792775240219908644239793785735715026873347600343865175952761926303160"
	" 3059144344244213709971259814753781636986470325476647558659373206291635324768958432433509563104347017837885763365758"
	" 1985150602287291935568054521177171638300868978215655730859378665066344726373823718423869104263333984641494340347905"
	" 927553665492332455747201965776037880757740193453592970025027978793976877002675564980949289727957565575433344219582",
	10);
#if 0
	printf("P=%s\n", P.getStr(16).c_str());
	printf("Q=%s\n", Q.getStr(16).c_str());
#endif
	GT e, e2;
	millerLoop(e, P, Q);
	finalExp(e, e);
//	printf("e\n%s\n", e.getStr(16).c_str());
	printf("check %s\n", e == g_blst_e ? "ok" : "ng");
	CYBOZU_BENCH_C("pairing", 1000, pairing, e, P, Q);
	CYBOZU_BENCH_C("ML", 1000, millerLoop, e, P, Q);
	CYBOZU_BENCH_C("FE", 1000, finalExp, e, e);
	{
		Fp a = e.a.a.a;
		Fp b = e.a.a.b;
		CYBOZU_BENCH_C("fp::mul", 10000, Fp::mul, a, a, b);
		CYBOZU_BENCH_C("fp::mul", 10000, Fp::mul, a, a, b);
	}
	{
		Fp2 a = e.a.a;
		Fp2 b = e.a.b;
		CYBOZU_BENCH_C("fp2::mul", 10000, Fp2::mul, a, a, b);
		CYBOZU_BENCH_C("fp2::sqr", 10000, Fp2::sqr, a, a);
	}
}

void mcl_bench()
{
	puts("--- use mcl ---");
	mcl_bench_sub();

	puts("--- use raw blst ---");
	using namespace mcl::bn;
	Fp::mul = (void (*)(Fp&, const Fp&, const Fp&))blst::blst_fp_mul;
	Fp::sqr = (void (*)(Fp&, const Fp&))blst::blst_fp_sqr;
	Fp2::mul = (void (*)(Fp2&, const Fp2&, const Fp2&))blst::blst_fp2_mul;
	Fp2::sqr = (void (*)(Fp2&, const Fp2&))blst::blst_fp2_sqr;
	mcl_bench_sub();
}

int main()
{
	mcl::bn::initPairing(mcl::BLS12_381);
	blst_bench();
	mcl_bench();
}

