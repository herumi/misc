#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <stdint.h>
#define USE_CPP11
#ifdef USE_CPP11
#include <random>
#endif


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
			p[i] = (1812433253 * (p[i-1] ^ (p[i-1] >> 30)) + i);
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

uint32_t history[624];

class MersenneTwisterNoLoop2 : public MersenneTwisterBase {
public:
	void init()
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
{
#if 1
#ifdef USE_CPP11
	std::random_device rd;
	std::mt19937 mt(rd());
	const int C = rd() % 123467;

	printf("C=%d\n", C);
	for (int i = 0; i < C; i++) {
		mt();
	}
	// get 624 dword data
	for (int i = 0; i < 624; i++) {
		history[i] = mt();
	}
	MersenneTwisterNoLoop2 mt2;
	mt2.init();
	// detect next value
	for (int i = 0; i < 100; i++) {
		uint32_t a = mt();
		uint32_t b = mt2.getInt32();
		if (a != b) {
			printf("err 0x%08x 0x%08x\n", a, b);
			exit(1);
		}
	}
	puts("ok");
#else
	const int SEED = 1234;
	MersenneTwisterNoLoop mt;
	mt.setSeed(SEED);
	const int C = 100000;

	for (int i = 0; i < C; i++) {
		mt.getInt32();
	}
	// get 624 dword data
	for (int i = 0; i < 624; i++) {
		history[i] = mt.getInt32();
	}
	MersenneTwisterNoLoop2 mt2;
	mt2.init();
	for (int i = 0; i < 10; i++) {
		uint32_t a = mt.getInt32();
		uint32_t b = mt2.getInt32();
		printf("0x%08x 0x%08x %c\n", a, b, a == b ? 'o' : 'x');
	}
#endif
#else
	const int SEED = 1234;
	MersenneTwisterOrg org;
	MersenneTwisterNoLoop nol;

	org.setSeed(SEED);
	nol.setSeed(SEED);

	uint32_t a, b;
	int i, j;

	printf("test of DNA\n");
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 4; j++) {
			a = org.getInt32();
			b = nol.getInt32();
			printf("%08x(%c)", a, (a == b) ? 'o' : 'x');
		}
		printf("\n");
	}

	org.setSeed(SEED);
	nol.setSeed(SEED);

	uint32_t reta = 0, retb = 0;
	for (i = 0; i < 10000000; i++) {
		reta += org.getInt32();
		retb += nol.getInt32();
	}
	printf("org=%08X\n", reta);
	printf("nol=%08X\n", retb);
	puts((reta == retb) ? "OK" : "NG");
#endif
}
