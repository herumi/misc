#include <iostream>
#include <cybozu/string.hpp>
#include <cybozu/string_operation.hpp>
#include <cybozu/mmap.hpp>
#include <stdlib.h>
#include <algorithm>
#include <map>
#include <vector>

struct IndexTable {
	typedef std::map<uint32_t, uint32_t> Map;
	typedef std::vector<uint32_t> Vec;
	Map valToIdx_;
	Vec idxToVal_;
	/*
		append value
		return index corresponding to value
	*/
	uint32_t append(uint32_t val)
	{
		std::pair<Map::iterator, bool> ret = valToIdx_.insert(Map::value_type(val, (uint32_t)valToIdx_.size()));
		if (!ret.second) {
			return ret.first->second;
		}
		idxToVal_.push_back(val);
		return (uint32_t)idxToVal_.size() - 1;
	}
	/*
		return index corresponding to value
	*/
	uint32_t getIdx(uint32_t val) const
	{
		Map::const_iterator i = valToIdx_.find(val);
		if (i != valToIdx_.end()) {
			return i->second;
		}
		printf("ERR no val in map %u\n", val);
		exit(1);
	}
	/*
		return value corresponding to index
	*/
	uint32_t getVal(uint32_t idx) const
	{
		if (idx < idxToVal_.size()) {
			return idxToVal_[idx];
		}
		printf("ERR too large idx %u\n", idx);
		exit(1);
	}
	void put() const
	{
		printf("map\n");
		for (Map::const_iterator i = valToIdx_.begin(), ie = valToIdx_.end(); i != ie; ++i) {
			printf("%u:%u ", i->first, i->second);
		}
		printf("\n");
		puts("tbl");
		for (size_t i = 0, ie = idxToVal_.size(); i < ie; i++) {
			printf("%u:%u ", (int)i, idxToVal_[i]);
		}
		printf("\n");
	}
};

struct Freq {
	int freq;
	cybozu::Char c;
	Freq(int freq = 0, cybozu::Char c = 0) : freq(freq), c(c) {}
	bool operator<(const Freq& rhs) const
	{
		if (freq < rhs.freq) return true;
		if (freq > rhs.freq) return false;
		return c < rhs.c;
	}
	bool operator>(const Freq& rhs) const
	{
		if (freq > rhs.freq) return true;
		if (freq < rhs.freq) return false;
		return c > rhs.c;
	}
};
typedef std::vector<Freq> FreqVec;

int main(int argc, char *argv[])
{
	argc--, argv++;
	if (argc == 0) return 1;
	typedef std::map<cybozu::Char, int> CharMap;
	CharMap cm;
	cybozu::Mmap m(*argv);
	cybozu::Utf8ref ref(m.get(), m.size());
//	IndexTable idxTbl;
	uint64_t charNum = 0;
	for (;;) {
		cybozu::Char c;
		if (!ref.next(&c)) break;
		cm[c]++;
		charNum++;
//		idxTbl.append(c);
	}
	printf("num=%d\n", (int)cm.size());
	FreqVec fv(cm.size());
	int pos = 0;
	for (CharMap::const_iterator i = cm.begin(), ie = cm.end(); i != ie; ++i) {
		fv[pos].c = i->first;
		fv[pos].freq = i->second;
		pos++;
	}
	std::sort(fv.begin(), fv.end(), std::greater<Freq>());
	const double rv = 100.0 / charNum;
	for (size_t i = 0, ie = fv.size(); i != ie; ++i) {
		char buf[5] = {};
		cybozu::Char c = fv[i].c;
		if (!cybozu::IsSpace(c)) {
			int len = cybozu::string::toUtf8(buf, c);
			if (len > 0) {
				buf[len] = 0;
			}
		}
		printf("%06x(%s) %8d %.2g%%\n", c, buf, fv[i].freq, fv[i].freq * rv);
	}
//	idxTbl.put();
}

