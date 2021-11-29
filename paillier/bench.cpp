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
	int bitLen;
	int adj;
	int n;
	void set(int sumNum, int bit, bool sign)
	{
		this->sumNum = sumNum;
		this->bitLen = bit + int(ceil(log2(sumNum)));
		this->adj = sign ? (1 << (bit - 1)) : 0;
		this->n = KEY_SIZE / this->bitLen;
	}
	void put() const
	{
		printf("sumNum=%d bitLen=%d adj=%d n=%d\n", sumNum, bitLen, adj, n);
	}
} g_p;

#define PUT(x) std::cout << std::hex << #x << '=' << x << std::endl;

uint64_t mask(int n)
{
	return (uint64_t(1) << n) - 1;
}

void pack(mpz_class& m, const IntVec& v)
{
	m = 0;
	for (int i = 0; i < g_p.n; i++) {
		m |= mpz_class(v[i] + g_p.adj) << (g_p.bitLen * i);
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
		v[i] = int(low & mask(g_p.bitLen)) - g_p.sumNum * g_p.adj;
		m >>= g_p.bitLen;
	}
}

using namespace mcl::paillier;

int main(int argc, char *argv[])
{
	cybozu::Option opt;
	int sumNum;
	int bit;
	bool sign;
	opt.appendOpt(&sumNum, 7, "sum", "max sum num");
	opt.appendOpt(&bit, 1, "bit", "bit per element");
	opt.appendBoolOpt(&sign, "sign", "true if element is signed");
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
		v[i] = short(rg.get32());
	}
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
