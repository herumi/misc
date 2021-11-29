/*
./a.out -sum 1000 -bit 16 -sign
enc  45.196Mclk
add  13.355Kclk
dec  23.229Mclk
*/
#include <vector>
#include <mcl/paillier.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <cybozu/option.hpp>
#include <iostream>
#include <math.h>

typedef std::vector<int> IntVec;

const int KEY_SIZE = 2048;

struct Param {
	int sumNum;
	int bit;
	int sign;
	int maxBitLen;
	int adj;
	int n;
	void set(int sumNum, int bit, bool sign)
	{
		this->sumNum = sumNum;
		this->bit = bit;
		this->sign = sign;
		this->maxBitLen = int(ceil(log2(((1 << bit) - 1) * sumNum)));
		this->adj = sign ? (1 << (bit - 1)) : 0;
		this->n = KEY_SIZE / this->maxBitLen;
	}
	void put() const
	{
		printf("sumNum=%d maxBitLen=%d adj=%d n=%d\n", sumNum, maxBitLen, adj, n);
	}
} g_p;

#define PUT(x) std::cout << std::hex << #x << '=' << x << std::endl;

uint64_t makeMask(int n)
{
	return (uint64_t(1) << n) - 1;
}

void pack(mpz_class& m, const IntVec& v)
{
	m = 0;
	for (int i = 0; i < g_p.n; i++) {
		m |= mpz_class(v[i] + g_p.adj) << (g_p.maxBitLen * i);
	}
}

void unpack(IntVec& v, mpz_class m)
{
	v.resize(g_p.n);
	uint64_t low;
	for (int i = 0; i < g_p.n; i++) {
		if (m == 0) {
			low = 0;
		} else {
			low = mcl::gmp::getUnit(m)[0];
		}
		v[i] = int(low & makeMask(g_p.maxBitLen)) - g_p.sumNum * g_p.adj;
		m >>= g_p.maxBitLen;
	}
}

using namespace mcl::paillier;

int main(int argc, char *argv[])
{
	cybozu::Option opt;
	int sumNum;
	int bit;
	bool sign;
	opt.appendOpt(&sumNum, 1000, "sum", "max sum num");
	opt.appendOpt(&bit, 16, "bit", "bit per element");
	opt.appendOpt(&sign, true, "sign", "true if element is signed");
	opt.appendHelp("h");
	if (!opt.parse(argc, argv)) {
		opt.put();
		return 1;
	}
	g_p.set(sumNum, bit, sign);
	g_p.put();
	cybozu::XorShift rg;
	SecretKey sec;
	sec.init(KEY_SIZE, rg);
	PublicKey pub;
	sec.getPublicKey(pub);
	IntVec v;
	v.resize(g_p.n);
	for (int i = 0; i < g_p.n; i++) {
		uint32_t x = rg.get32() & makeMask(g_p.bit);
		if (g_p.sign) {
			v[i] = int(x) - (1 << (g_p.bit - 1));
		} else {
			v[i] = x;
		}
	}
	printf("top of v ");
	for (int i = 0; i < 16; i++) {
		printf("%d ", v[i]);
	}
	printf("\n");
	mpz_class m;
	pack(m, v);
	mpz_class c;
	pub.enc(c, m);

	mpz_class cs = c;
	for (int i = 1; i < g_p.sumNum; i++) {
		pub.add(cs, cs, c);
	}

	mpz_class d;
	sec.dec(d, cs);
	IntVec dv;
	unpack(dv, d);
	for (int i = 0; i < g_p.n; i++) {
		if (dv[i] != v[i] * g_p.sumNum) {
			printf("ERR i=%d dv=%d v=%d\n", i, dv[i], v[i]);
			return 1;
		}
	}
	puts("ok");
	CYBOZU_BENCH_C("enc", 100, pub.enc, c, m);
	CYBOZU_BENCH_C("add", 100, pub.add, cs, cs, c);
	CYBOZU_BENCH_C("dec", 100, sec.dec, d, cs);
}
