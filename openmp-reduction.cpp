/*
	g++ openmp-reduction.cpp -O2 -fopenmp
*/
#define MCL_USE_VINT
#include <mcl/she.hpp>
#include <cybozu/benchmark.hpp>
using namespace mcl::she;

typedef std::vector<CipherTextG1> CTVec;

void sumVec1(CipherTextG1& r, const CTVec& cv)
{
	r.clear();
	for (size_t i = 0; i < cv.size(); i++) {
		add(r, r, cv[i]);
	}
}

void sumVec2(CipherTextG1& r, const CTVec& cv)
{
	r.clear();
	#pragma omp declare reduction(add:CipherTextG1:add(omp_out, omp_out, omp_in)) initializer(omp_priv = omp_orig)
	#pragma omp parallel for reduction(add:r)
	for (size_t i = 0; i < cv.size(); i++) {
		add(r, r, cv[i]);
	}
}

int main()
{
	const size_t N = 1000;
	const size_t hashSize = 65536;
	initG1only(mcl::ecparam::secp256k1, hashSize);
	SecretKey sec;
	sec.setByCSPRNG();
	PublicKey pub;
	sec.getPublicKey(pub);
	PrecomputedPublicKey ppub;
	ppub.init(pub);
	CTVec cv(N);
	for (size_t i = 0; i < N; i++) {
		ppub.enc(cv[i], 1);
	}
	CipherTextG1 r1, r2;
	CYBOZU_BENCH_C("sumVec1", 100, sumVec1, r1, cv);
	CYBOZU_BENCH_C("sumVec2", 100, sumVec2, r2, cv);
	int d = sec.dec(r1);
	printf("d=%d\n", d);
	d = sec.dec(r2);
	printf("d=%d\n", d);
}
