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
struct MapToG2onBLS12_381 {
	Fp2 xi_2;
	Fp2 Ell2p_a;
	Fp2 Ell2p_b;
	Fp half;
	void init()
	{
		xi_2.a = 1;
		xi_2.b = 1;
		Ell2p_a.a = 0;
		Ell2p_a.b = 240;
		Ell2p_b.a = 1012;
		Ell2p_b.b = 1012;
		half = -1;
		half /= 2;
	}
	bool osswu2_help(Fp2& x0, Fp2& y0, const Fp2& t) const
	{
		Fp2 num_den_common;
		Fp2::mul_xi(num_den_common, t);
		Fp2::sqr(num_den_common, num_den_common);
		num_den_common += xi_2;
		Fp2 t2;
		Fp2::sqr(t2, t);
		num_den_common *= t2; // = ((t * xi)^2 + xi) * t^2
		if (num_den_common.isZero()) {
			x0 = Ell2p_b / (xi_2 * Ell2p_a);
		} else {
			x0 = -Ell2p_b * (num_den_common + 1) / (Ell2p_a * num_den_common);
		}
		Fp2 gx0;
		Fp2::sqr(gx0, x0);
		gx0 += Ell2p_a;
		gx0 *= x0;
		gx0 += Ell2p_b;
		if (Fp2::squareRoot(y0, gx0)) {
			if (!((t.a > half) ^ (y0.a > half))) {
				Fp2::neg(y0, y0);
			}
puts("AAA");
			return true;
		}
		Fp2 xi_t2;
		Fp2::mul_xi(xi_t2, t2);
		x0 *= xi_t2; // (xi t^2) x0
		Fp2 gx1;
		Fp2::sqr(gx1, xi_t2);
		gx1 *= xi_t2;
		gx1 *= gx0; // (xi t^2)^3 gx0
		if (!Fp2::squareRoot(y0, gx1)) {
			throw cybozu::Exception("bad");
			return false;
		}
		if (t.a <= half) {
			Fp2::neg(y0, y0);
		}
puts("BB");
		return true;
	}
};

