#pragma once
#include <cybozu/endian.hpp>

namespace util {

inline uint32_t rotl(uint32_t x, int s)
{
	return (x << s) | (x >> (32 - s));
}

inline void qr(uint32_t state[16], int i0, int i1, int i2, int i3)
{
	uint32_t a = state[i0];
	uint32_t b = state[i1];
	uint32_t c = state[i2];
	uint32_t d = state[i3];
	a += b; d ^= a; d = rotl(d, 16);
	c += d; b ^= c; b = rotl(b, 12);
	a += b; d ^= a; d = rotl(d, 8);
	c += d; b ^= c; b = rotl(b, 7);
	state[i0] = a;
	state[i1] = b;
	state[i2] = c;
	state[i3] = d;
}

inline void update(uint32_t state[16])
{
	qr(state, 0, 4, 8, 12);
	qr(state, 1, 5, 9, 13);
	qr(state, 2, 6, 10, 14);
	qr(state, 3, 7, 11, 15);
	qr(state, 0, 5, 10, 15);
	qr(state, 1, 6, 11, 12);
	qr(state, 2, 7, 8, 13);
	qr(state, 3, 4, 9, 14);
}

}

class ChaCha20 {
	uint32_t state_[16];
	uint32_t k_[8];
	uint32_t n_[3];
	uint32_t b_;
public:
	void initState()
	{
		state_[0] = 0x61707865;
		state_[1] = 0x3320646e;
		state_[2] = 0x79622d32;
		state_[3] = 0x6b206574;
		for (int i = 0; i < 8; i++) {
			state_[4 + i] = k_[i];
		}
		state_[12] = b_;
		for (int i = 0; i < 3; i++) {
			state_[13 + i] = n_[i];
		}
	}
	void init(const uint8_t key[32], const uint8_t nonce[12])
	{
		for (int i = 0; i < 8; i++) {
			k_[i] = cybozu::Get32bitAsLE(&key[i * 4]);
		}
		for (int i = 0; i < 3; i++) {
			n_[i] = cybozu::Get32bitAsLE(&nonce[i * 4]);
		}
		b_ = 1;
	}
	void enc64(uint8_t *dst, const uint8_t *src)
	{
		initState();
		uint32_t keep[16];
		for (int i = 0; i < 16; i++) {
			keep[i] = state_[i];
		}
		for (int i = 0; i < 10; i++) {
			util::update(state_);
		}
		for (int i = 0; i < 16; i++) {
			state_[i] += keep[i];
		}
		for (int i = 0; i < 16; i++) {
			uint32_t v = cybozu::Get32bitAsLE(&src[i * 4]);
			v ^= state_[i];
			cybozu::Set32bitAsLE(&dst[i * 4], v);
		}
		b_++;
	}
	const uint32_t *getState() const { return &state_[0]; }
};
