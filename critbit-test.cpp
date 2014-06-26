#include "critbit.hpp"
#include <string>
#include <stdlib.h>
#include <set>
#include <vector>
#define CYBOZU_TEST_DISABLE_AUTO_RUN
#include <cybozu/test.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/mmap.hpp>

struct StrSet : std::set<std::string> {
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
void bench(const char *msg, const std::string& text)
{
	printf("bench:%s\n", msg);
	std::vector<std::string> sampling;
	Set set;
	std::istringstream iss(text);
	std::string word;
	int c = 0;
	while (iss >> word) {
		set.insert(word);
		c++;
		if ((c % 100) == 0 && sampling.size() < 10) {
			sampling.push_back(word);
		}
	}
	printf("word num %d\n", (int)set.size());
	printf("sampling num %d\n", (int)sampling.size());
	for (size_t i = 0; i < sampling.size(); i++) {
		const std::string w = sampling[i];
		const char *msg = sampling[i].c_str();
		CYBOZU_BENCH(msg, set.has, w);
	}
	const std::string str = "maybe-not-found-string";
	CYBOZU_BENCH(str.c_str(), set.has, str);
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
		bench<critbit::StrSet>("critbit::set", text);
		bench<StrSet>("std::set", text);
		bench<critbit::StrSet>("critbit::set", text);
	}
	return cybozu::test::autoRun.run(argc, argv);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
