#include <iostream>
#include <string>
#include <map>
#include <fstream>

int main(int argc, char *argv[])
{
	if (argc == 1) return 1;
	std::fstream ifs(argv[1], std::ios::binary);
	std::string s;
	typedef std::map<std::string, int> Str2Int;
	Str2Int m;
	while (ifs >> s) {
		m[s]++;
	}
	for (Str2Int::const_iterator i = m.begin(), ie = m.end(); i != ie; ++i) {
		printf("%s %d\n", i->first.c_str(), i->second);
	}
}
