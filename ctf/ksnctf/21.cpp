#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <stdint.h>
#include <cybozu/mmap.hpp>
#include <cybozu/endian.hpp>
#include <cybozu/file.hpp>

class MersenneTwisterBase {
protected:
	enum {
		N = 624,
		M = 397,
		MATRIX_A = 0x9908B0DF,   /* constant vector a */
		UMASK = 0x80000000, /* most significant w-r bits */
		LMASK = 0x7FFFFFFF /* least significant r bits */
	};
	inline uint32_t MIXBITS(uint32_t u, uint32_t v)
	{
		return (u & UMASK) | (v & LMASK);
	}
	inline uint32_t TWIST(uint32_t u, uint32_t v)
	{
		return (MIXBITS(u, v) >> 1) ^ ((v & 1) ? MATRIX_A : 0u);
	}

	/**
		init sate_
		@param seed [in] random seq
		@param seedNum [in] random num
	*/
	void setState(const uint32_t *seed, int seedNum)
	{
		memset(state_, 0, sizeof(state_));

		if (seedNum <= 0) {
			state_[0] = 1234;
			seedNum = 1;
		} else {
			if ((size_t)seedNum > N) seedNum = N;
			memcpy(state_, seed, seedNum * sizeof(state_[0]));
		}

		uint32_t *const p = state_;
		for (size_t i = seedNum; i < N; i++) {
			p[i] = (1812433253 * (p[i-1] ^ (p[i-1] >> 30)) + uint32_t(i));
		}
	}
	uint32_t index_;
	uint32_t state_[N];

	MersenneTwisterBase()
		: index_(0)
	{
	}
public:
	void putState() const
	{
		printf("index=%d\n", index_);
		for (size_t i = 0; i < N; i++) {
			printf("0x%08x", state_[i]);
			if (i < N - 1) printf(", ");
			if ((i & 7) == 7) putchar('\n');
		}
		printf("\n");
	}
};


class MersenneTwisterOrg : public MersenneTwisterBase {
private:
	uint32_t output_[N];
	void next()
	{
		uint32_t *p, *q;

		p = state_;
		for (size_t i = 0; i < N - M; i++, p++) {
			*p = p[M] ^ TWIST(p[0], p[1]);
		}

		for (size_t i = 0; i < M - 1; i++, p++) {
			*p = p[M-N] ^ TWIST(p[0], p[1]);
		}

		*p = p[M-N] ^ TWIST(p[0], state_[0]);
		index_ = 0;

		p = state_;
		q = output_;
		for (size_t i = 0; i < N; i++) {
			uint32_t y;
			y = *p++;
			/* Tempering */
			y ^= (y >> 11);
			y ^= (y << 7) & 0x9D2C5680;
			y ^= (y << 15) & 0xEFC60000;
			y ^= (y >> 18);
			*q++ = y;
		}
	}
public:
	void setSeed(const uint32_t *seed, int seedNum)
	{
		setState(seed, seedNum);
		next();
	}

	void setSeed(uint32_t seed)
	{
		setSeed(&seed, 1);
	}

	inline uint32_t getInt32()
	{
		if (index_ == N) {
			next();
		}
		return output_[index_++];
	}
};

uint32_t tempering(uint32_t x)
{
	x ^= (x >> 11);
	x ^= (x << 7) & 0x9D2C5680;
	x ^= (x << 15) & 0xEFC60000;
	x ^= (x >> 18);
	return x;
}
uint32_t mask(int x)
{
	return (1 << x) - 1;
}

uint32_t rev_tempering(uint32_t x)
{
	x ^= x >> 18;
	x ^= (x << 15) & 0xEFC60000;

	//	x ^= (x << 7) & 0x9D2C5680;
	{
		// x = [f:(e:d):c:b:a] = [4:(3:4):7:7:7]
		// (x << 7) = [d:c:b:a:0]
		const uint32_t w = 0x9D2C5680;
		uint32_t a = x & mask(7);
		uint32_t b = ((x ^ ((a << 7) & w)) >> 7) & mask(7);
		uint32_t c = ((x ^ ((b << 14) & w)) >> 14) & mask(7);
		uint32_t de = ((x ^ ((c << 21) & w)) >> 21) & mask(7);
		uint32_t d = de & mask(4);
		uint32_t f = (x ^ ((d << 28) & w)) >> 28;
		x = a | (b << 7) | (c << 14) | (de << 21) | (f << 28);
	}

	// x ^= x >> 11;
	{
		// x = [c:b:a] = [10:11:11]
#if 1
		x ^= x >> 11;
		x ^= x >> 22;
#else
		uint32_t c = x >> 22;
		uint32_t b = (x >> 11) ^ c;
		uint32_t a = (x ^ b) & mask(11);
		x = a | (b << 11) | (c << 22);
#endif
	}
	return x;
}

class MersenneTwisterNoLoop : public MersenneTwisterBase {
public:
	void setSeed(const uint32_t *seed, int seedNum)
	{
		setState(seed, seedNum);
		index_ = 0;
	}

	void setSeed(uint32_t seed)
	{
		setSeed(&seed, 1);
	}

	inline uint32_t getInt32()
	{
		uint32_t i = index_;
		uint32_t *const p = state_;
		uint32_t y;
		y = p[i] = p[(i + M) % N] ^ TWIST(p[i], p[(i + 1) % N]);

		index_ = (i + 1) % N;
		return tempering(y);
	}
};


class MersenneTwisterNoLoop2 : public MersenneTwisterBase {
public:
	void init(uint32_t history[624])
	{
		for (size_t i = 0; i < N; i++) {
			state_[i] = rev_tempering(history[i]);
		}
		index_ = 0;
	}

	inline uint32_t getInt32()
	{
		uint32_t i = index_;
		uint32_t *const p = state_;
		uint32_t y;
		y = p[i] = p[(i + M) % N] ^ TWIST(p[i], p[(i + 1) % N]);

		index_ = (i + 1) % N;
		return tempering(y);
	}
};

int main()
	try
{
	cybozu::Mmap mCpp("encrypt/encrypt.cpp");
	cybozu::Mmap mEnc("encrypt/encrypt.enc");
	const uint32_t size1 = cybozu::Get32bitAsLE(mEnc.get());

	// get 624 dword data
	const int N = 624;
	uint32_t history[N];
	for (int i = 0; i < N; i++) {
		uint32_t a = cybozu::Get32bitAsLE(mCpp.get() + i * 4);
		uint32_t b = cybozu::Get32bitAsLE(mEnc.get() + i * 4 + 4);
		history[i] = a ^ b;
	}
	MersenneTwisterNoLoop2 mt2;
	mt2.init(history);
	cybozu::Mmap mFlag("encrypt/flag.enc");

	const char *decName = "flag.dec";
	{
		cybozu::File ofs;
		ofs.openW(decName);
		const uint32_t size2 = cybozu::Get32bitAsLE(mFlag.get());
		printf("dec size=%d\n", size2);

		for (uint32_t i = 0; i < size1 / 4 - N; i++) {
			mt2.getInt32();
		}
		for (uint32_t i = 0; i < size2; i += 4) {
			char buf[4];
			uint32_t a = cybozu::Get32bitAsLE(mFlag.get() + i + 4);
			uint32_t b = mt2.getInt32();
			cybozu::Set32bitAsLE(buf, a ^ b);
			ofs.write(buf, 4);
		}
	}
	cybozu::Mmap mDec(decName);
	for (int i = 0x107a; i < 0x10a4; i += 2) {
		putchar(mDec.get()[i]);
	}
	printf("\n");
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}

