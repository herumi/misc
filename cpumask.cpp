#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <cybozu/test.hpp>

/*
	a_ is treated as an array of N elements, each being bitN bits
	a_ = 1<<bitN and n_ = 0 and range_ = 0 means empty set
	n_ is length of a_[] - 1
	When range_ is false (discrete values):
		Values satisfy a_[i] + 1 < a_[i+1] for all 0 <= i <= n_
	When range_ is true (intervals):
		v = a_[i*2] is the start of the interval
		n = a_[i*2+1] is the interval length - 1
		Represents the interval [v, v+n]
	Max number of cpu = 2**bitN - 1
	Max value that can be stored = N
	Max interval length = N/2
*/
template<uint32_t N = 6, uint32_t bitN = 10>
struct CpuMaskT {
	static const uint64_t mask = (uint64_t(1) << bitN) - 1;
	uint64_t a_:N*bitN;
	uint64_t n_:3;
	uint64_t range_:1;

	// Set a_[idx] = v
	void set_a(size_t idx, uint32_t v)
	{
		assert(idx < N);
		assert(v <= mask);
		a_ &= ~(mask << (idx*bitN));
		a_ |= (v & mask) << (idx*bitN);
	}
	// Get a_[idx]
	uint32_t get_a(size_t idx) const
	{
		assert(idx < N);
		return (a_ >> (idx*bitN)) & mask;
	}
public:
	CpuMaskT() : a_(1<<bitN), n_(0), range_(0) {}
	bool empty() const
	{
		return a_ == 1 << bitN && n_ == 0 && range_ == 0;
	}
	// Add element v
	bool append(uint32_t v)
	{
		if (v > mask) return false;
		// When adding for the first time, treat as discrete value
		if (empty()) {
			a_ = v;
			n_ = 0;
			return true;
		}
		if (!range_) {
			// If there's one discrete value and it forms an interval with the new value, switch to interval mode
			if (n_ == 0 && get_a(0) + 1 == v) {
				set_a(1, 1);
				range_ = 1;
				n_ = 1;
				return true;
			}
			if (n_ >= N - 1) return false;
			// Add discrete value
			n_++;
			set_a(n_, v);
			return true;
		}
		uint32_t n = get_a(n_);
		// If the value to add is 1 greater than the end of the current interval
		if (get_a(n_ - 1) + n + 1 == v) {
			// Increase the interval length by one
			set_a(n_, n + 1);
			return true;
		} else {
			if (n_ >= N - 1) return false;
			// If not continuous with the previous interval
			// Add a new interval [v]
			set_a(n_ + 1, v);
			n_ += 2;
			return true;
		}
	}
	// Return true if the idx-th value exists
	bool hasNext(uint32_t idx) const
	{
		if (empty()) return false;
		if (!range_) return idx <= n_;
		uint32_t n = 0;
		for (uint32_t i = 1; i <= n_; i += 2) {
			n += get_a(i) + 1;
			if (idx < n) return true;
		}
		return false;
	}
	uint32_t get(uint32_t idx) const
	{
		assert(hasNext(idx));
		if (!range_) return get_a(idx);
		uint32_t n = 0;
		for (uint32_t i = 1; i <= n_; i += 2) {
			uint32_t range = get_a(i) + 1;
			if (idx < n + range) {
				return get_a(i - 1) + (idx - n);
			}
			n += range;
		}
		return false;
	}
	size_t size() const
	{
		if (empty()) return 0;
		if (!range_) return n_ + 1;
		size_t n = 0;
		for (uint32_t i = 1; i <= n_; i += 2) {
			n += get_a(i) + 1;
		}
		return n;
	}
	void dump() const
	{
		printf("a_:");
		for (int i = int(N) - 1; i >= 0; i--) {
			printf("%u ", uint32_t((a_ >> (i * bitN)) & mask));
		}
		printf("\n");
		printf("n_: %u\n", (uint32_t)n_);
		printf("range_: %u\n", (uint32_t)range_);
	}
	void put() const
	{
		if (empty()) {
			printf("empty\n");
			return;
		}
		if (!range_) {
			for (uint32_t i = 0; i <= n_; i++) {
				printf("%u ", get_a(i));
			}
			printf("\n");
			return;
		}
		for (uint32_t i = 0; i <= n_; i += 2) {
			uint32_t v = get_a(i);
			uint32_t len = get_a(i + 1);
			if (len == 0) {
				printf("[%u] ", v);
			} else {
				printf("[%u-%u] ", v, v + len);
			}
		}
		printf("\n");
	}
};

typedef CpuMaskT<> CpuMask;

uint32_t popcnt(uint32_t v)
{
	uint32_t n = 0;
	while (v) {
		if (v & 1) n++;
		v >>= 1;
	}
	return n;
}

CYBOZU_TEST_AUTO(pattern)
{
	const uint32_t N = 8;
	const uint32_t bitN = 3;
	const uint32_t bit = 1 << bitN;
	typedef CpuMaskT<N, bitN> TestMask;
	for (uint32_t i = 0; i < (1 << bit); i++) {
		TestMask m;
		uint32_t cnt = 0;
		for (uint32_t j = 0; j < bit; j++) {
			if (i & (1 << j)) {
				cnt++;
				CYBOZU_TEST_ASSERT(m.append(j));
			}
		}
		CYBOZU_TEST_EQUAL(m.size(), cnt);
#if 0
		printf("pattern (%3u) ", i);
		for (int j = int(bit) - 1; j >= 0; j--) {
			if (i & (uint64_t(1) << j)) printf("%d ", j);
		}
		printf("\n");
		m.dump();
		m.put();
#endif
		uint32_t idx = 0;
		while (m.hasNext(idx)) {
			uint32_t v = m.get(idx);
			CYBOZU_TEST_ASSERT(i & (1 << v));
			idx++;
		}
		CYBOZU_TEST_EQUAL(idx, cnt);
	}
}
