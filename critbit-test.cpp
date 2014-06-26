#include "critbit.hpp"
#include <string>
#include <stdlib.h>
#include <set>
#include <vector>
#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/mmap.hpp>
#include <cybozu/unordered_set.hpp>

typedef std::vector<std::string> StrVec;

struct StrSet : std::set<std::string> {
	bool has(const std::string& str) const
	{
		return this->find(str) != this->end();
	}
};
struct StrHash : CYBOZU_NAMESPACE_STD::unordered_set<std::string> {
	bool has(const std::string& str) const
	{
		return this->find(str) != this->end();
	}
};

CYBOZU_TEST_AUTO(has)
{
	critbit::StrSet crit;
	static const char* elems[] = {"a", "aa", "b", "bb", "ab", "ba", "aba", "bab", NULL};

	for (size_t i = 0; elems[i]; ++i) {
		crit.insert(elems[i]);
	}
	CYBOZU_TEST_EQUAL(crit.size(), 8);

	for (size_t i = 0; elems[i]; ++i) {
		CYBOZU_TEST_ASSERT(crit.has(elems[i]));
	}
}

CYBOZU_TEST_AUTO(remove)
{
	static const char* elems[] = {"a", "aa", "b", "bb", "ab", "ba", "aba", "bab", NULL};

	for (size_t i = 1; elems[i]; ++i) {
		critbit::StrSet crit;
		for (size_t j = 0; j < i; ++j) crit.insert(elems[j]);
		CYBOZU_TEST_EQUAL(crit.size(), i);

		for (size_t j = 0; j < i; ++j) {
			CYBOZU_TEST_ASSERT(crit.has(elems[j]));
		}

		for (size_t j = 0; j < i; ++j) {
			CYBOZU_TEST_ASSERT(crit.remove(elems[j]));
		}
		CYBOZU_TEST_EQUAL(crit.size(), 0);

		for (size_t j = 0; j < i; ++j) {
			CYBOZU_TEST_ASSERT(!crit.has(elems[j]));
		}
	}
}

static bool handler(void *s, const char* elem)
{
	((StrSet*)s)->insert(elem);
	return true;
}

CYBOZU_TEST_AUTO(traverse)
{
	critbit::StrSet crit;
	static const char* elems[] = {"a", "aa", "aaz", "abz", "bba", "bbc", "bbd", NULL};

	for (size_t i = 0; elems[i]; ++i) crit.insert(elems[i]);
	CYBOZU_TEST_EQUAL(crit.size(), 7);

	StrSet s;
	crit.traverse("a", &s, handler);

	CYBOZU_TEST_EQUAL(s.size(), 4);
	CYBOZU_TEST_ASSERT(s.find("a") != s.end());
	CYBOZU_TEST_ASSERT(s.find("aa") != s.end());
	CYBOZU_TEST_ASSERT(s.find("aaz") != s.end());
	CYBOZU_TEST_ASSERT(s.find("abz") != s.end());

	s.clear();
	crit.traverse("aa", &s, handler);

	CYBOZU_TEST_EQUAL(s.size(), 2);
	CYBOZU_TEST_ASSERT(s.find("aa") != s.end());
	CYBOZU_TEST_ASSERT(s.find("aaz") != s.end());
}

template<class Set>
int findAll(const Set& s, const StrVec& sv)
{
	int c = 0;
	for (size_t i = 0; i < sv.size(); i++) {
		c += s.has(sv[i]);
	}
	return c;
}

template<class Set>
void bench(const char *msg, const std::string& text)
{
	printf("bench:%s -------------------\n", msg);
	StrVec words;
	Set set;
	std::istringstream iss(text);
	std::string word;
	while (iss >> word) {
		set.insert(word);
		words.push_back(word);
	}
	printf("word num %d\n", (int)set.size());
	for (size_t i = 0; i < set.size(); i++) {
		const std::string w = words[i];
		const char *msg = words[i].c_str();
		CYBOZU_BENCH(msg, set.has, w);
		if (i == 9) break;
	}
	const std::string str = "maybe-not-found-string";
	CYBOZU_BENCH(str.c_str(), set.has, str);
	int c = 0;
	CYBOZU_BENCH_C("findAll", 1, c = findAll, set, words);
	printf("count=%d\n", c);
}

int main(int argc, char *argv[])
	try
{
	if (argc == 2) {
		const std::string file = argv[1];
		printf("load file %s\n", file.c_str());
		cybozu::Mmap m(file);
		const std::string text(m.get(), m.size());
		bench<StrSet>("std::set", text);
		bench<StrHash>("std::unordered_set", text);
		bench<critbit::StrSet>("critbit::set", text);
	}
	return cybozu::test::autoRun.run(argc, argv);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
