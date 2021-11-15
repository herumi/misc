#include <vector>
#include <mcl/paillier.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <iostream>

typedef std::vector<int> IntVec;

const int KEY_SIZE = 2048;
const int SUM_NUM = 1000;
const size_t L = 16 + 10; /* SHORT_MAX * SUM_NUM */
const int ADJ = 32768;
const size_t N = KEY_SIZE / L;

#define PUT(x) std::cout << std::hex << #x << '=' << x << std::endl;

uint64_t mask(int n)
{
	return (uint64_t(1) << n) - 1;
}

void pack(mpz_class& m, const IntVec& v)
{
	m = 0;
	for (size_t i = 0; i < N; i++) {
		m |= mpz_class(v[i] + ADJ) << (L * i);
	}
}

void unpack(IntVec& v, mpz_class m)
{
	v.resize(N);
	uint64_t low;
	for (size_t i = 0; i < N; i++) {
		if (m == 0) {
			low = 0;
		} else {
			low = mcl::gmp::getUnit(m)[0];
		}
		v[i] = int(low & mask(L)) - SUM_NUM * ADJ;
		m >>= L;
	}
}

using namespace mcl::paillier;

int main()
{
	cybozu::XorShift rg;
	SecretKey sec;
	sec.init(KEY_SIZE, rg);
	PublicKey pub;
	sec.getPublicKey(pub);
	IntVec v;
	v.resize(N);
	for (size_t i = 0; i < N; i++) {
		v[i] = short(rg.get32());
	}
	mpz_class m;
	pack(m, v);
	mpz_class c;
	pub.enc(c, m);

	mpz_class cs = c;
	for (int i = 1; i < SUM_NUM; i++) {
		pub.add(cs, cs, c);
	}

	mpz_class d;
	sec.dec(d, cs);
	IntVec dv;
	unpack(dv, d);
	for (size_t i = 0; i < N; i++) {
		if (dv[i] != v[i] * SUM_NUM) {
			printf("ERR i=%zd dv=%d v=%d\n", i, dv[i], v[i]);
			return 1;
		}
	}
	puts("ok");
	printf("N=%zd\n", N);
	CYBOZU_BENCH_C("enc", 100, pub.enc, c, m);
	CYBOZU_BENCH_C("add", 100, pub.add, cs, cs, c);
	CYBOZU_BENCH_C("dec", 100, sec.dec, d, cs);
}
