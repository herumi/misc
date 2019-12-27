#pragma once
/**
	@file
	@brief map to G2 on BLS12-381
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
	ref. https://eprint.iacr.org/2019/403 , https://github.com/kwantam/bls12-381_hash
*/

// clear_h2 == mulByCofactorBLS12fast
struct MapToG2_WB19 {
	Fp2 xi;
	Fp2 Ell2p_a;
	Fp2 Ell2p_b;
	Fp half;
	mpz_class sqrtConst; // (p^2 - 9) / 16
	Fp2 root4[4];
	Fp2 etas[4];
	void init()
	{
		bool b;
		xi.a = -2;
		xi.b = -1;
		Ell2p_a.a = 0;
		Ell2p_a.b = 240;
		Ell2p_b.a = 1012;
		Ell2p_b.b = 1012;
		half = -1;
		half /= 2;
		sqrtConst = Fp::getOp().mp;
		sqrtConst *= sqrtConst;
		sqrtConst -= 9;
		sqrtConst /= 16;
		const char *rv1Str = "0x6af0e0437ff400b6831e36d6bd17ffe48395dabc2d3435e77f76e17009241c5ee67992f72ec05f4c81084fbede3cc09";
		root4[0].a = 1;
		root4[0].b.clear();
		root4[1].a.clear();
		root4[1].b = 1;
		root4[2].a.setStr(&b, rv1Str);
		assert(b); (void)b;
		root4[2].b = root4[2].a;
		root4[3].a = root4[2].a;
		Fp::neg(root4[3].b, root4[3].a);
		const char *ev1Str = "0x699be3b8c6870965e5bf892ad5d2cc7b0e85a117402dfd83b7f4a947e02d978498255a2aaec0ac627b5afbdf1bf1c90";
		const char *ev2Str = "0x8157cd83046453f5dd0972b6e3949e4288020b5b8a9cc99ca07e27089a2ce2436d965026adad3ef7baba37f2183e9b5";
		const char *ev3Str = "0xab1c2ffdd6c253ca155231eb3e71ba044fd562f6f72bc5bad5ec46a0b7a3b0247cf08ce6c6317f40edbc653a72dee17";
		const char *ev4Str = "0xaa404866706722864480885d68ad0ccac1967c7544b447873cc37e0181271e006df72162a3d3e0287bf597fbf7f8fc1";
		Fp& ev1 = etas[0].a;
		Fp& ev2 = etas[0].b;
		Fp& ev3 = etas[2].a;
		Fp& ev4 = etas[2].b;
		ev1.setStr(&b, ev1Str);
		assert(b); (void)b;
		ev2.setStr(&b, ev2Str);
		assert(b); (void)b;
		Fp::neg(etas[1].a, ev2);
		etas[1].b = ev1;
		ev3.setStr(&b, ev3Str);
		assert(b); (void)b;
		ev4.setStr(&b, ev4Str);
		assert(b); (void)b;
		Fp::neg(etas[3].a, ev4);
		etas[3].b = ev3;
	}
	/*
		(a+bi)*(-2-i) = (b-2a)-(a+2b)i
	*/
	void mul_xi(Fp2& y, const Fp2& x) const
	{
	}
	bool isNegSign(const Fp2& x) const
	{
		if (x.b > half) return true;
		if (!x.b.isZero()) return false;
		if (x.a > half) return true;
		if (!x.b.isZero()) return false;
		return false;
	}
	bool osswu2_help(Fp2& x0, Fp2& y0, Fp2& z0, const Fp2& t) const
	{
		printf("t=%s\n", t.getStr(16).c_str());
		Fp2 t2, t2xi;
		Fp2::sqr(t2, t);
		Fp2 den, den2;
		Fp2::mul(t2xi, t2, xi);
		den = t2xi;
		Fp2::sqr(den2, den);
		// (t^2 * xi)^2 + (t^2 * xi)
		den += den2;
		Fp2 x0_num, x0_den;
		Fp2::add(x0_num, den, 1);
		x0_num *= Ell2p_b;
		if (den.isZero()) {
			Fp2::mul(x0_den, Ell2p_a, xi);
		} else {
			Fp2::mul(x0_den, -Ell2p_a, den);
		}
		Fp2 x0_den2, x0_den3, gx0_den, gx0_num;
		Fp2::sqr(x0_den2, x0_den);
		Fp2::mul(x0_den3, x0_den2, x0_den);
		gx0_den = x0_den3;

		Fp2::mul(gx0_num, Ell2p_b, gx0_den);
		Fp2 tmp, tmp1, tmp2;
		Fp2::mul(tmp, Ell2p_a, x0_num);
		tmp *= x0_den2;
		gx0_num += tmp;
		Fp2::sqr(tmp, x0_num);
		tmp *= x0_num;
		gx0_num += tmp;

		Fp2::sqr(tmp1, gx0_den); // x^2
		Fp2::sqr(tmp2, tmp1); // x^4
		tmp1 *= tmp2;
		tmp1 *= gx0_den; // x^7
		Fp2::mul(tmp2, gx0_num, tmp1);
		tmp1 *= tmp2;
		tmp1 *= gx0_den;
		Fp2 candi;
		Fp2::pow(candi, tmp1, sqrtConst);
		candi *= tmp2;
		bool isNegT = isNegSign(t);
		for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(root4); i++) {
			Fp2::mul(y0, candi, root4[i]);
			Fp2::sqr(tmp, y0);
			tmp *= gx0_den;
			if (tmp == gx0_num) {
				if (isNegSign(y0) != isNegT) {
					Fp2::neg(y0, y0);
				}
				Fp2::mul(x0, x0_num, x0_den);
				y0 *= x0_den3;
				z0 = x0_den;
// nocheck
				return true;
			}
		}
		Fp2 x1_num, x1_den, gx1_num, gx1_den;
		Fp2::mul(x1_num, t2xi, x0_num);
		x1_den = x0_den;
		Fp2::mul(gx1_num, den2, t2xi);
		gx1_num *= gx0_num;
		gx1_den = gx0_den;
		candi *= t2;
		candi *= t;
// ok
		for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(etas); i++) {
			Fp2::mul(y0, candi, etas[i]);
			Fp2::sqr(tmp, y0);
			tmp *= gx1_den;
			if (tmp == gx1_num) {
				if (isNegSign(y0) != isNegT) {
					Fp2::neg(y0, y0);
				}
				Fp2::mul(x0, x1_num, x1_den);
				Fp2::sqr(tmp, x1_den);
				y0 *= tmp;
				y0 *= x1_den;
				z0 = x1_den;
				return true;
			}
		}
		return false;
	}
};

