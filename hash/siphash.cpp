#include <siphash.hpp>
#include "org_siphash.c"
#include "MurmurHash3.cpp"
#include <cybozu/option.hpp>
#include <cybozu/mmap.hpp>

void run(const std::string& name, const std::string& data)
{
	uint64_t v;
	if (name == "org_sip") {
		char key[16] = {};
		v = siphash24(data.data(), data.size(), key);
		printf("v=%016llx\n", (long long)v);
	} else if (name == "sip") {
		v = cybozu::siphash24(data.data(), data.size());
		printf("v=%016llx\n", (long long)v);
	} else if (name == "mur") {
		uint64_t v[2];
		MurmurHash3_x64_128(data.data(), data.size(), 0, v);
		printf("v=%016llx:%016llx\n", (long long)v[0], (long long)v[1]);
	} else {
		printf("not support hash %s\n", name.c_str());
		return;
	}
}

int main(int argc, char *argv[])
	try
{
	std::string hash;
	std::string file;
	size_t size;
	cybozu::Option opt;
	opt.appendOpt(&hash, "sip", "n", ": name of hash(org_sip|sip)");
	opt.appendOpt(&file, "", "f", ": select file");
	opt.appendOpt(&size, 0, "s", ": data size");
	opt.appendHelp("h");
	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 1;
	}
	if (file.empty() ^ (size != 0)) {
		printf("specify each -f or -s\n");
		opt.usage();
		return 1;
	}
	std::string data;
	if (size > 0) {
		data.resize(size);
		for (size_t i = 0; i < size; i++) {
			data[i] = (char)i;
		}
	} else {
		cybozu::Mmap map(file);
		data.assign(map.get(), map.size());
	}
	run(hash, data);
} catch (cybozu::Exception& e) {
	printf("ERR %s\n", e.what());
}

