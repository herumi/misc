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
		z = x * y;
		reduction(z);
		if (z >= p_) {
			z -= p_;
		}
	}
	void reduction(mpz_class& z) const
	{
#if 0
		const size_t N = 72 / sizeof(BlockType);
		BlockType buf[N];
		const size_t zn = mie::Gmp::getBlockSize(z);
		assert(zn  <= pn_ * 2 && pn_ * 2 <= N);
		memcpy(buf, mie::Gmp::getBlock(p), zn * sizeof(BlockType));
		memset(buf[zn], 0, (pn_ - zn) * sizeof(BlockType));
		reduction(buf);
#else
		for (size_t i = 0; i < pn_; i++) {
			BlockType q = mie::Gmp::getBlock(z, 0) * pp_;
			z += p_ * q;
			z >>= sizeof(BlockType) * 8;
		}
#endif
	}
#if 0
	void mullAdd(BlockType *z, const BlockType *x, BlockType y)
	void reduction(BlockType *z) const
	{
		for (size_t i = 0; i < pn_; i++) {
			BlockType q = z[i] * pp_;
			z += p_ * q;
			z >>= sizeof(BlockType) * 8;
		}
	}
#endif
};

