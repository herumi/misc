#include <set>
#include <stdio.h>
#include <map>

int main()
{
#ifdef CPP17
	std::set s = { 1, 5, 9, 2, 4 };
	auto [i, inserted] = s.insert(2);
	if (inserted) {
		printf("inseted %d\n", *i);
	} else {
		puts("not inserted");
	}
	std::map<std::string, int> m = { { "abc", 3 },  { "xyz", 4 }, { "qqq",  2 } };
	for (const auto& [key, val] : m) {
		printf("%s %d\n", key.c_str(), val);
	}
#else
	std::set<int> s = { 1, 5, 9, 2, 4 };
	auto i = s.insert(22);
	if (i.second) {
		printf("inseted %d\n", *i.first);
	} else {
		puts("not inserted");
	}
#endif
}
