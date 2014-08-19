#include <memory>
#include <stdio.h>
#include <cxxabi.h>
#include <cstdlib>
#include <iostream>
#include <fstream>

std::string demangler(const std::string& name)
{
	int status;
	char* demangled = abi::__cxa_demangle(name.c_str(), 0, 0, &status);

	if (status == 0) {
		std::string ret = demangled;
		std::free(demangled);
		return ret;
	}
	return "";
}
bool acceptChar(char c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_';
}
// _ZN7HScript7BS_ExecEPKc
void demangleIfFound(std::string& line)
{
	size_t begin = line.find('_');

	if (begin == std::string::npos) return;
	size_t q = begin + 1;
	while (q < line.size()) {
		if (!acceptChar(line[q])) break;
		q++;
	}
	std::string demangled = demangler(line.substr(begin, q - begin));
	if (demangled.empty()) return;
	line = line.substr(0, begin) + demangled + line.substr(q);
}

int main(int argc, char *argv[])
{
	std::istream *pis;
	std::ifstream ifs;
	if (argc == 1) {
		pis = &std::cin;
	} else {
		ifs.open(argv[1]);
		pis = &ifs;
	}
	std::string line;
	while (std::getline(*pis, line)) {
		demangleIfFound(line);
		printf("%s\n", line.c_str());
	}
}
