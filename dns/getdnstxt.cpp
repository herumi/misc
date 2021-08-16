/*
	g++ getdnstxt.cpp -O2 -Wall -Wextra -lresolv
	nslookup -type=TXT domain
*/
#include <stdio.h>
#include <resolv.h>
#include <string>
#include <vector>

void getDnsTxt(std::vector<std::string>& txts, const std::string& domain)
{
	uint8_t buf[4096];
	int len = res_query(domain.c_str(), C_IN, ns_t_txt, buf, sizeof(buf));
	if (len < 0) {
		perror("res_query");
		return;
	}
	ns_msg msg;
	int ret = ns_initparse(buf, len, &msg);
	if (ret < 0) {
		perror("ns_initparse");
		return;
	}
	for (int i = 0; i < ns_msg_count(msg, ns_s_an); i++) {
		ns_rr rr;
		if (ns_parserr(&msg, ns_s_an, i, &rr) < 0) {
			perror("ns_parserr");
			return;
		}
		ns_type type = ns_rr_type(rr);
		if (type == ns_t_txt) {
			const uint8_t *p = ns_rr_rdata(rr);
			int n = *p;
			txts.push_back(std::string((const char*)p + 1, n));
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "domain\n");
		return 1;
	}
	const std::string domain = argv[1];
	std::vector<std::string> txts;
	res_init();
	getDnsTxt(txts, domain);
	for (size_t i = 0; i < txts.size(); i++) {
		printf("%s\n", txts[i].c_str());
	}
}
