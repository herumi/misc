#pragma once

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <assert.h>

class Sha1 {
	uint64_t totalSize_;
	size_t roundBufSize_;
	char roundBuf_[64];
	static const size_t Hsize = 5;
	uint32_t H_[Hsize];
	uint32_t K_[80];

	uint32_t get32bitAsBE(const void *src) const
	{
		const uint8_t *p = (const uint8_t*)src;
		return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
	}
	void set32bitAsBE(void *dst, uint32_t v) const
	{
		uint8_t *p = (uint8_t*)dst;
		p[0] = uint8_t(v >> 24);
		p[1] = uint8_t(v >> 16);
		p[2] = uint8_t(v >> 8);
		p[3] = uint8_t(v);
	}

	uint32_t S(uint32_t x, int s) const
	{
#ifdef _MSC_VER
		return _rotl(x, s);
#else
		return (x << s) | (x >> (32 - s));
#endif
	}

	uint32_t f0(uint32_t b, uint32_t c, uint32_t d) const { return (b & c) | (~b & d); }
	uint32_t f1(uint32_t b, uint32_t c, uint32_t d) const { return b ^ c ^ d; }
	uint32_t f2(uint32_t b, uint32_t c, uint32_t d) const { return (b & c) | (b & d) | (c & d); }
	uint32_t f(int t, uint32_t b, uint32_t c, uint32_t d) const
	{
		if (t < 20) {
			return f0(b, c, d);
		} else
		if (t < 40) {
			return f1(b, c, d);
		} else
		if (t < 60) {
			return f2(b, c, d);
		} else {
			return f1(b, c, d);
		}
	}

	void reset()
	{
		static const uint32_t tbl[] = {
			0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6
		};
		for (size_t i = 0; i < 4; i++) {
			for (int j = 0; j < 20; j++) {
				K_[i * 20 + j] = tbl[i];
			}
		}
		totalSize_ = 0;
		roundBufSize_ = 0;
		H_[0] = 0x67452301;
		H_[1] = 0xefcdab89;
		H_[2] = 0x98badcfe;
		H_[3] = 0x10325476;
		H_[4] = 0xc3d2e1f0;
	}
	/**
		@param buf [in] buffer(64byte)
	*/
public:
	void round(const char *buf)
	{
		uint32_t W[80];
		for (int i = 0; i < 16; i++) {
			W[i] = get32bitAsBE(&buf[i * 4]);
		}
		for (int i = 16 ; i < 80; i++) {
			W[i] = S(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);
		}
		uint32_t a = H_[0];
		uint32_t b = H_[1];
		uint32_t c = H_[2];
		uint32_t d = H_[3];
		uint32_t e = H_[4];
		for (int i = 0; i < 80; i++) {
			uint32_t tmp = S(a, 5) + f(i, b, c, d) + e + W[i] + K_[i];
			e = d;
			d = c;
			c = S(b, 30);
			b = a;
			a = tmp;
		}
		H_[0] += a;
		H_[1] += b;
		H_[2] += c;
		H_[3] += d;
		H_[4] += e;
		totalSize_ += 64;
	}
	/*
		final phase
		@note bufSize < 64
	*/
	void term(const char *buf, size_t bufSize)
	{
		assert(bufSize < 64);
		const uint64_t totalSize = totalSize_ + bufSize;

		uint8_t last[64];
		memcpy(last, buf, bufSize);
		memset(&last[bufSize], 0, 64 - bufSize);
		last[bufSize] = uint8_t(0x80); /* top bit = 1 */
		if (bufSize >= 56) {
			round((const char*)last);
		}
		set32bitAsBE(&last[56], uint32_t(totalSize >> 29));
		set32bitAsBE(&last[60], uint32_t(totalSize * 8));
		round((const char*)last);
	}
public:
	Sha1()
	{
		reset();
	}
	void update(const char *buf, size_t bufSize)
	{
		if (bufSize == 0) return;
		if (roundBufSize_ > 0) {
			size_t size = std::min(64 - roundBufSize_, bufSize);
			memcpy(roundBuf_ + roundBufSize_, buf, size);
			roundBufSize_ += size;
			buf += size;
			bufSize -= size;
		}
		if (roundBufSize_ == 64) {
			round(roundBuf_);
			roundBufSize_ = 0;
		}
		while (bufSize >= 64) {
			assert(roundBufSize_ == 0);
			round(buf);
			buf += 64;
			bufSize -= 64;
		}
		if (bufSize > 0) {
			assert(bufSize < 64);
			assert(roundBufSize_ == 0);
			memcpy(roundBuf_, buf, bufSize);
			roundBufSize_ = bufSize;
		}
		assert(roundBufSize_ < 64);
	}
	void update(const std::string& buf)
	{
		update(buf.c_str(), buf.size());
	}
	std::string digest(const char *buf, size_t bufSize)
	{
		update(buf, bufSize);
		term(roundBuf_, roundBufSize_);
		std::string ret =  get();
		reset();
		return ret;
	}
	std::string digest(const std::string& str = "")
	{
		return digest(str.c_str(), str.size());
	}
	void set(const std::string& d, size_t len)
	{
		if (d.size() != 20 || len != 64) {
			printf("set not support %d %d\n", (int)d.size(), (int)len);
			exit(1);
		}
		const uint32_t *in = (const uint32_t*)d.c_str();
		for (size_t i = 0; i < Hsize; i++) {
			set32bitAsBE(&H_[i], in[i]);
		}
		totalSize_ = len;
	}
	std::string get() const
	{
		std::string s;
		s.resize(20);
		uint32_t *p = (uint32_t*)&s[0];
		for (size_t i = 0; i < Hsize; i++) {
			set32bitAsBE(&p[i], H_[i]);
		}
		return s;
	}
};
