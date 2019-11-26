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
	mpz_class sqrtConst;
	Fp2 tbl[4];
	Fp2 tbl2[4];
	void init()
	{
		bool b;
		xi.a = 1;
		xi.b = 1;
		Ell2p_a.a = 0;
		Ell2p_a.b = 240;
		Ell2p_b.a = 1012;
		Ell2p_b.b = 1012;
		half = -1;
		half /= 2;
		sqrtConst = Fp::getOp().mp;
		sqrtConst *= sqrtConst;
		sqrtConst += 7;
		sqrtConst /= 16;
		Fp rv1;
		rv1.setStr(&b, "0x6af0e0437ff400b6831e36d6bd17ffe48395dabc2d3435e77f76e17009241c5ee67992f72ec05f4c81084fbede3cc09");
		assert(b); (void)b;
		tbl[0].a = 1;
		tbl[0].b = 0;
		tbl[1].a = 0;
		tbl[1].b = 1;
		tbl[2].a = rv1;
		tbl[2].b = rv1; // tbl[2]^2 = -i
		tbl[3].a = rv1;
		tbl[3].b = -rv1; // tbl[3]^2 = i
		Fp ev1, ev2;
		ev1.setStr(&b, "0x2c4a7244a026bd3e305cc456ad9e235ed85f8b53954258ec8186bb3d4eccef7c4ee7b8d4b9e063a6c88d0aa3e03ba01");
		assert(b); (void)b;
		ev2.setStr(&b, "0x85fa8cd9105715e641892a0f9a4bb2912b58b8d32f26594c60679cc7973076dc6638358daf3514d6426a813ae01f51a");
		assert(b); (void)b;
		tbl2[0].a = ev1;
		tbl2[0].b = 0; // tbl2[0]^4 = -8
		tbl2[1].a = 0;
		tbl2[1].b = ev1; // tbl2[1]^4 = -8
		tbl2[2].a = ev2;
		tbl2[2].b = ev2; // tbl2[2]^4 = 8
		tbl2[3].a = ev2;
		tbl2[3].b = -ev2; // tbl2[3]^4 = 8
	}
	bool osswu2_help(Fp2& x0, Fp2& y0, const Fp2& t) const
	{
		Fp2 t2;
		Fp2::sqr(t2, t);
		Fp2 den;
		/*
			xi^2 = (1 + i)^2 = 2i
		*/
		// (a + bi)i = -b + ai
		Fp::neg(den.a, t2.b);
		den.b = t2.a;
		den += den;
		den += xi;
		den *= t2;
		/*
			den = ((t * xi)^2 + xi) * t^2
			= (t^2 * (xi^2) + xi) * t^2
		*/
		if (den.isZero()) {
			// x0 = Ell2p_b / (Ell2p_a * xi)
			Fp2::mul_xi(x0, Ell2p_a);
			Fp2::div(x0, Ell2p_b, x0);
		} else {
			x0 = -Ell2p_b * (den + 1) / (Ell2p_a * den);
		}
		Fp2 gx0;
		Fp2::sqr(gx0, x0);
		gx0 += Ell2p_a;
		gx0 *= x0;
		gx0 += Ell2p_b;
		Fp2 candi;
		Fp2::pow(candi, gx0, sqrtConst);
		for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
			Fp2::mul(y0, candi, tbl[i]);
			Fp2 y2;
			Fp2::sqr(y2, y0);
			if (y2 == gx0) {
				if (t.a > half) {
					Fp2::neg(y0, y0);
				}
				return true;
			}
		}
		Fp2 xi_t2;
		Fp2::mul_xi(xi_t2, t2);
		x0 *= xi_t2; // (xi t^2) x0
		Fp2 gx1;
		Fp2::sqr(gx1, xi_t2);
		gx1 *= xi_t2;
		gx1 *= gx0; // (xi t^2)^3 gx0
		candi *= t2;
		candi *= t;
		for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl2); i++) {
			Fp2::mul(y0, candi, tbl2[i]);
			Fp2 y2;
			Fp2::sqr(y2, y0);
			if (y2 == gx1) {
				return true;
			}
		}
		return false;
	}
};

