#include <crypt.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <string.h>
#include <stdlib.h>

typedef std::vector<std::string> StrVec;

struct Info {
	std::string user;
	std::string salt;
	std::string hash;
	void put() const
	{
		printf("user=%s, salt=%s, hash=%s\n", user.c_str(), salt.c_str(), hash.c_str());
	}
};

typedef std::vector<Info> InfoVec;

int main(int argc, char *argv[])
	try
{
	argc--, argv++;
	if (argc < 1) {
		printf("%s dict-file pass-file\n", argv[-1]);
		return 1;
	}
	std::ifstream dicFs(argv[0], std::ios::binary);
	StrVec dict;
	std::string s;
	while (dicFs >> s) {
		dict.push_back(s);
	}
	InfoVec iv;
	std::ifstream passFs(argv[1], std::ios::binary);
	while (passFs >> s) {
		const char *p1, *p2, *p3;
		p1 = strchr(s.c_str(), ':');
		p2 = strchr(p1 + 2, '$');
		if (p2) {
			p2 = strchr(p2 + 1, '$');
		}
		p3 = strchr(p2 + 1, ':');
		if (!p1 || !p2 || !p3) {
			printf("parse err %s\n", s.c_str());
			continue;
		}
		Info info;
		info.user.assign(s.c_str(), p1);
		info.salt.assign(p1 + 1, p2);
		info.hash.assign(p2 + 1, p3);
		info.put();
		iv.push_back(info);
	}
	for (size_t i = 0; i < iv.size(); i++) {
		const Info& info = iv[i];
//		info.put();
		for (size_t j = 0; j < dict.size(); j++) {
			const std::string& pass = dict[j];
			const char *v = crypt(pass.c_str(), info.salt.c_str());
			if (v == 0) {
				continue;
			}
			if (info.hash == v + info.salt.size() + 1) {
				printf("user=%s, pass=%s\n", info.user.c_str(), pass.c_str());
			}
		}
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
