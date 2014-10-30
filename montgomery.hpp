#include <mie/gmp_util.hpp>
#include <mie/mont_fp.hpp>

struct Montgomery {
	typedef mie::Gmp::BlockType BlockType;
	mpz_class p_;
	mpz_class R_; // (1 << (pn_ * 64)) % p
	mpz_class RR_; // (R * R) % p
	BlockType pp_; // p * pp = -1 mod M = 1 << 64
	size_t pn_;
	Montgomery() {}
	explicit Montgomery(const mpz_class& p)
	{
		p_ = p;
		pp_ = mie::montgomery::getCoff(mie::Gmp::getBlock(p, 0));
		pn_ = mie::Gmp::getBlockSize(p);
		R_ = 1;
		R_ = (R_ << (pn_ * 64)) % p_;
		RR_ = (R_ * R_) % p_;
	}

	void toMont(mpz_class& z, const mpz_class& x) const { mul(z, x, RR_); }
	void fromMont(mpz_class& z, const mpz_class& x) const { mul(z, x, 1); }

	void mul(mpz_class& z, const mpz_class& x, const mpz_class& y) const
	{
#if 0
		const size_t ySize = mie::Gmp::getBlockSize(y);
		mpz_class c = x * mie::Gmp::getBlock(y, 0);
		BlockType q = mie::Gmp::getBlock(c, 0) * pp_;
		c += p_ * q;
		c >>= sizeof(BlockType) * 8;
		for (size_t i = 1; i < pn_; i++) {
			if (i < ySize) {
				c += x * mie::Gmp::getBlock(y, i);
			}
			BlockType q = mie::Gmp::getBlock(c, 0) * pp_;
			c += p_ * q;
			c >>= sizeof(BlockType) * 8;
		}
		if (c >= p_) {
			c -= p_;
		}
		z = c;
#else
		z = x * y;
		for (size_t i = 0; i < pn_; i++) {
			BlockType q = mie::Gmp::getBlock(z, 0) * pp_;
			z += p_ * q;
			z >>= sizeof(BlockType) * 8;
		}
		if (z >= p_) {
			z -= p_;
		}
#endif
	}
};

