#include <gmpxx.h>
#include <stdio.h>
#include <iostream>
#include <cybozu/bit_operation.hpp>
#include <cybozu/benchmark.hpp>
#include <algorithm>

template<typename Unit, int N, typename INT>
struct InvModT {
	static const int modL = 62;
	static const INT MASK = (INT(1) << modL) - 1;
	mpz_class M;
	INT Mi;
	struct Tmp {
		INT u, v, q, r;
	};

	INT divsteps_n_matrix(Tmp& t, INT eta, INT f, INT g) const
	{
		static const int tbl[] = { 15, 5, 3, 9, 7, 13, 11, 1 };
		INT u = 1, v = 0, q = 0, r = 1;
		int i = modL;
		for (;;) {
			INT zeros = (std::min<int>)(i, cybozu::bsf(g));
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

	void update_de(mpz_class& d, mpz_class& e, const Tmp& t, const mpz_class& M, INT Mi) const
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
		INT cd0 = d.get_si() + M.get_si() * md;
		INT ce0 = e.get_si() + M.get_si() * me;
		md -= (Mi * cd0) & MASK;
		me -= (Mi * ce0) & MASK;
		d += M * md;
		e += M * me;
		d >>= modL;
		e >>= modL;
	}

	void normalize(mpz_class& v, bool minus, const mpz_class& M) const
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

	void invMod(mpz_class& z, const mpz_class& x, const mpz_class& m) const
	{
		mpz_invert(z.get_mpz_t(), x.get_mpz_t(), m.get_mpz_t());
	}

	INT getMod2powN(const mpz_class& x) const
	{
		INT a = x.get_si();
		return a & MASK;
	}

	void modinv(mpz_class& o, const mpz_class& M, INT Mi, const mpz_class& x) const
	{
		INT eta = -1;
		mpz_class f = M;
		mpz_class g = x;
		mpz_class d = 0;
		mpz_class e = 1;
		Tmp t;
		while (g != 0) {
			eta = divsteps_n_matrix(t, eta, getMod2powN(f), getMod2powN(g));
			update_fg(f, g, t);
			update_de(d, e, t, M, Mi);
		}
		normalize(d, f < 0, M);
		o = d;
	}
	void init(const char *Mstr)
	{
		printf("M=%s\n", Mstr);
		M.set_str(Mstr, 16);
		mpz_class inv;
		mpz_class mod = mpz_class(1) << modL;
		invMod(inv, M, mod);
		Mi = inv.get_si();
		printf("Mi=%lld\n", (long long)Mi);
	}
	void inv(mpz_class& y, const mpz_class& x) const
	{
		modinv(y, M, Mi, x);
	}
};

template<int N>
void test(const char *Mstr)
{
	InvModT<long, N, long> invMod;
	invMod.init(Mstr);
	mpz_class x, y;
	x = 1;
	for (int i = 0; i < 10000; i++) {
		invMod.inv(y, x);
		if ((x * y) % invMod.M != 1) {
			std::cout << "err" << std::endl;
			std::cout << "x=" << x << std::endl;
			std::cout << "y=" << y << std::endl;
			return;
		}
		x = y + 1;
	}
	puts("ok");
	CYBOZU_BENCH_C("modinv", 1000, x++;invMod.inv, x, x);
}

#include "main.hpp"
