#include <iostream>
#include <string>
#include <map>
#include <fstream>

int main(int argc, char *argv[])
{
	if (argc == 1) return 1;
	std::ifstream ifs(argv[1], std::ios::binary);
	std::map<std::string, int> m;
	std::string s;
	while (ifs >> s) {
		m[s]++;
	}
	for (const auto& e : m) {
		printf("%s %d\n", e.first.c_str(), e.second);
	}
}
