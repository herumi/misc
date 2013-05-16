#include <map>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>

typedef std::vector<uint32_t> Vec;

struct AscendVec0 {
	Vec v_;
	void append(uint32_t pos)
	{
		assert(v_.empty() ? pos == 0 : v_.back() < pos);
		v_.push_back(pos);
	}
	uint32_t get(uint32_t pos) const
	{
		const uint32_t n = (uint32_t)v_.size();
		assert(n > 0 && v_[0] == 0);
		for (uint32_t i = 1; i < n; i++) {
			if (pos < v_[i]) return i - 1;
		}
		return n - 1;
	}
};

struct AscendVec1 {
	typedef std::map<uint32_t, uint32_t> Map;
	Map m_;

	void append(uint32_t pos)
	{
		assert(m_.empty() ? pos == 0 : (--m_.end())->first < pos);
		const uint32_t idx = (uint32_t)m_.size();
		m_[pos] = idx;
	}
	uint32_t get(uint32_t pos) const
	{
		assert(!m_.empty() && m_.begin()->first == 0);
		Map::const_iterator i = m_.lower_bound(pos + 1);
		if (i == m_.end()) return (uint32_t)m_.size() - 1;
		return i->second - 1;
	}
};

struct AscendVec2 {
	Vec v_;
	void append(uint32_t pos)
	{
		assert(v_.empty() ? pos == 0 : v_.back() < pos);
		v_.push_back(pos);
	}
	uint32_t get(uint32_t pos) const
	{
		assert(!v_.empty() && v_[0] == 0);
		Vec::const_iterator i = std::lower_bound(v_.begin(), v_.end(), pos + 1);
		if (i == v_.end()) return (uint32_t)v_.size() - 1;
		return (uint32_t)std::distance(v_.begin(), i) - 1;
	}
};

template<class AV>
void test()
{
	const uint32_t data[] = {
		0, 3, 5, 6, 9, 13
	};
	AV av;
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(data); i++) {
		av.append(data[i]);
	}
	const struct {
		uint32_t pos;
		uint32_t v;
	} tbl[] = {
		{ 0, 0 },
		{ 1, 0 },
		{ 2, 0 },
		{ 3, 1 },
		{ 4, 1 },
		{ 5, 2 },
		{ 6, 3 },
		{ 7, 3 },
		{ 8, 3 },
		{ 9, 4 },
		{10, 4 },
		{11, 4 },
		{12, 4 },
		{13, 5 },
		{14, 5 },
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		CYBOZU_TEST_EQUAL(av.get(tbl[i].pos), tbl[i].v);
	}
}

CYBOZU_TEST_AUTO(0) { test<AscendVec0>(); }
CYBOZU_TEST_AUTO(1) { test<AscendVec1>(); }
CYBOZU_TEST_AUTO(2) { test<AscendVec2>(); }

template<class AV>
struct Bench {
	cybozu::XorShift rg_;
	AV av_;
	uint32_t pos_;
	uint32_t sum_;
	Bench(const char *msg)
		: pos_(0)
		, sum_(0)
	{
		const int N = 200000;
		av_.append(0);
		uint32_t maxV = 0;
		for (int i = 0; i< N; i++) {
			uint32_t v = rg_.get32();
			if (v > 0xe0000000) {
				v %= 80000;
			} else {
				v %= 800;
			}
			if (pos_ > 0xffffffff - v) {
				printf("too large %d %u %u\n", i, pos_, v);
				break;
			}
			v++; // avoid v = 0
			if (v > maxV) maxV = v;
			pos_ += v;
			av_.append(pos_);
		}
		printf("count=%u, max=%u, pos=%u, ave=%.2f\n", N, maxV, pos_, pos_ / double(N));
		CYBOZU_BENCH_C(msg, 100000, run);
	}
	void run()
	{
		uint32_t v = rg_.get32() % pos_;
		uint32_t a = av_.get(v);
		sum_ += a;
	}
	~Bench()
	{
		printf("sum=%d\n", sum_);
	}
};

CYBOZU_TEST_AUTO(bench)
{
	Bench<AscendVec0>("vec seq   ");
	Bench<AscendVec1>("map       ");
	Bench<AscendVec2>("bin search");
}
