#include <gmpxx.h>
#include <stdio.h>
#include <iostream>
#include <cybozu/bit_operation.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/test.hpp>
#include <algorithm>

typedef __int128_t int128_t;

namespace mcl { namespace gmp {
void invMod(mpz_class& z, const mpz_class& x, const mpz_class& m)
{
	mpz_invert(z.get_mpz_t(), x.get_mpz_t(), m.get_mpz_t());
}

} }

template<int N, typename INT, typename UINT>
struct InvModT {
	static const int modL = 62;
	static const INT modN = INT(1) << modL;
	static const INT half = modN / 2;
	static const INT MASK = modN - 1;
	mpz_class M;
	INT Mi;
	struct Tmp {
		INT u, v, q, r;
		std::string getStr() const
		{
			char buf[256];
			snprintf(buf, sizeof(buf), "(%ld, %ld, %ld, %ld)", u, v, q, r);
			return buf;
		}
	};
	INT adj(INT x) const
	{
		x &= MASK;
		if (x >= half) return x - modN;
		return x;
	}

	INT divsteps_n_matrix(Tmp& t, INT eta, INT f, INT g) const
	{
		static const int tbl[] = { 15, 5, 3, 9, 7, 13, 11, 1 };
		INT u = 1, v = 0, q = 0, r = 1;
		int i = modL;
		for (;;) {
			INT zeros = g == 0 ? modL : cybozu::bsf(g);
			if (i < zeros) zeros = i;
			eta -= zeros;
			i -= zeros;
			g >>= zeros;
			u <<= zeros;
			v <<= zeros;
			if (i == 0) break;
			if (eta < 0) {
				INT u0 = u;
				INT v0 = v;
				INT f0 = f;
				eta = -eta;
				f = g;
				u = q;
				v = r;
				g = -f0;
				q = -u0;
				r = -v0;
			}
			int limit = (std::min<INT>)((std::min<INT>)(eta + 1, i), 4);
			INT w = (g * tbl[(f & 15)>>1]) & ((1<<limit)-1);
			g += w * f;
			q += w * u;
			r += w * v;
		}
		t.u = u;
		t.v = v;
		t.q = q;
		t.r = r;
		return eta;
	}

	void update_fg(mpz_class& f, mpz_class& g, const Tmp& t) const
	{
		mpz_class f0 = f;
		f = f * t.u + g * t.v;
		g = f0 * t.q + g * t.r;
		f >>= modL;
		g >>= modL;
	}

	INT signMod(UINT x) const
	{
		x &= (UINT(1) << (modL + 1)) - 1;
		if (x >= half) {
			return INT(x) - modN;
		} else {
			return INT(x);
		}
	}

	void update_de(mpz_class& d, mpz_class& e, const Tmp& t) const
	{
		INT md = 0;
		INT me = 0;
		if (d < 0) {
			md += t.u;
			me += t.q;
		}
		if (e < 0) {
			md += t.v;
			me += t.r;
		}
		mpz_class d0 = d;
		d = d * t.u + e * t.v;
		e = d0 * t.q + e * t.r;
		INT di = getLow(d) + getLow(M) * md;
		INT ei = getLow(e) + getLow(M) * me;
		md -= Mi * di;
		me -= Mi * ei;
		md &= MASK;
		me &= MASK;
		if (md >= half) md -= modN;
		if (me >= half) me -= modN;
		d += M * md;
		e += M * me;
		d >>= modL;
		e >>= modL;
#if 0
		if (d >= M) {
			d -= M;
		}
#endif
	}

	void normalize(mpz_class& v, bool minus) const
	{
		if (v < 0) {
			v += M;
		}
		if (minus) {
			v = -v;
		}
		if (v < 0) {
			v += M;
		}
	}

	INT getLow(const mpz_class& x) const
	{
		INT r = x.get_mpz_t()->_mp_d[0];
//		r &= MASK;
		if (x < 0) r = -r;
		return r;
	}
	UINT getUlow(const mpz_class& x) const
	{
		UINT r = x.get_mpz_t()->_mp_d[0];
		if (x < 0) r = -r;
		return r;
	}
	INT getLowMask(const mpz_class& x) const
	{
		INT r = getLow(x);
		return r & MASK;
	}

	void inv(mpz_class& o, const mpz_class& x) const
	{
//		std::cout << "inp " << x << std::endl;
		INT eta = -1;
		mpz_class f = M;
		mpz_class g = x;
		mpz_class d = 0;
		mpz_class e = 1;
		Tmp t;
		while (g != 0) {
			eta = divsteps_n_matrix(t, eta, getLow(f), getLow(g));
//			std::cout << "B " << eta << " " << t.getStr() << std::endl;
			update_fg(f, g, t);
//			std::cout << "C " << f << " " << g << " " << t.getStr() << std::endl;
			update_de(d, e, t);
//			std::cout << "D " << d << " " << e << std::endl;
		}
		normalize(d, f < 0);
		o = d;
	}
	void init(const char *Mstr)
	{
		M.set_str(Mstr, 16);
		mpz_class inv;
		mpz_class mod = mpz_class(1) << modL;
		mcl::gmp::invMod(inv, M, mod);
		Mi = getLowMask(inv);
	}
};

template<class INV>
void check(const INV& invMod, const mpz_class& M);

template<int N>
void test(const char *Mstr)
{
	InvModT<N, long, unsigned long> invMod;
	invMod.init(Mstr);
	std::cout << "M " << invMod.M << std::endl;
	printf("Mi %lld\n", (long long)invMod.Mi);
	check(invMod, invMod.M);
}

#include "main.hpp"
