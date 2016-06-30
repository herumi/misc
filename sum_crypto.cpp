/*
	homo add crypto sample
	cl sum_crypto.cpp -I../mcl/include -DMCL_NO_AUTOLINK ../mcl/src/fp.cpp
*/
#include <iostream>
#include <fstream>
#include <cybozu/random_generator.hpp>
#include <cybozu/option.hpp>
#include <cybozu/itoa.hpp>
#include <mcl/fp.hpp>
#include <mcl/ec.hpp>
#include <mcl/elgamal.hpp>
#include <mcl/ecparam.hpp>

struct TagZn;
typedef mcl::FpT<> Fp;
typedef mcl::FpT<TagZn> Zn; // use ZnTag because Zn is different class with Fp
typedef mcl::EcT<Fp> Ec;
typedef mcl::ElgamalT<Ec, Zn> Elgamal;

typedef std::vector<int> IntVec;

cybozu::RandomGenerator rg;

const std::string pubFile = "sum_pub.txt";
const std::string prvFile = "sum_prv.txt";
const std::string resultFile = "sum_ret.txt";

std::string GetSheetName(size_t n)
{
	return std::string("sum_") + cybozu::itoa(n) + ".txt";
}

struct Param {
	std::string mode;
	IntVec iv;
	Param(int argc, const char *const argv[])
	{
		cybozu::Option opt;
		opt.appendVec(&iv, "l", ": list of int(eg. 1 5 3 2)");
		opt.appendHelp("h", ": put this message");
		opt.appendParam(&mode, "mode", ": init/enc/sum/dec");
		if (!opt.parse(argc, argv)) {
			opt.usage();
			exit(1);
		}
	}
};

void SysInit()
{
	const mcl::EcParam& para = mcl::ecparam::secp192k1;
	Zn::setModulo(para.n);
	Fp::setModulo(para.p);
	Ec::setParam(para.a, para.b);
	Ec::setCompressedExpression(true);
}

template<class T>
bool Load(T& t, const std::string& name, bool doThrow = true)
{
	std::ifstream ifs(name.c_str(), std::ios::binary);
	if (!ifs) {
		if (doThrow) throw cybozu::Exception("Load:can't read") << name;
		return false;
	}
	if (ifs >> t) return true;
	if (doThrow) throw cybozu::Exception("Load:bad data") << name;
	return false;
}

template<class T>
void Save(const std::string& name, const T& t)
{
	std::ofstream ofs(name.c_str(), std::ios::binary);
	ofs << t;
}

void Init()
{
	puts("init");
	const mcl::EcParam& para = mcl::ecparam::secp192k1;
	const Fp x0(para.gx);
	const Fp y0(para.gy);
	const Ec P(x0, y0);
	const size_t bitSize = para.bitSize;

	Elgamal::PrivateKey prv;
	prv.init(P, bitSize, rg);
	const Elgamal::PublicKey& pub = prv.getPublicKey();
	printf("make privateKey=%s, publicKey=%s\n", prvFile.c_str(), pubFile.c_str());
	Save(prvFile, prv);
	Save(pubFile, pub);
}

void Enc(const IntVec& iv)
{
	Elgamal::PublicKey pub;
	Load(pub, pubFile);
	for (size_t i = 0; i < iv.size(); i++) {
		Elgamal::CipherText c;
		pub.enc(c, iv[i], rg);
		std::cout << '"' << c << '"';
		if (i < iv.size() - 1) {
			std::cout << ',';
		}
	}
}

void Sum()
{
	Elgamal::PublicKey pub;
	Load(pub, pubFile);
	Elgamal::CipherText result;
	std::string line;
	while (std::getline(std::cin, line, ',')) {
		if (!line.empty() && line[line.size() - 1] == '\n') {
			line.resize(line.size() - 1);
		}
		const char *begin = line.c_str();
		const char *end = begin + line.size();
		if (begin != end) {
			if (*begin == '"') begin++;
			if (begin != end) {
				if (end[-1] == '"') end--;
				if (begin != end) {
					Elgamal::CipherText c;
					std::string s(begin, end);
					std::istringstream is(s);
					if (is >> c) {
						result.add(c);
					}
				}
			}
		}
	}
	std::cout << result << std::endl;
}

void Dec()
{
	Elgamal::PrivateKey prv;
	Load(prv, prvFile);
	prv.setCache(0, 10000);
	Elgamal::CipherText c;
	std::cin >> c;
	int sum = prv.dec(c);
	std::cout << sum << std::endl;
}

int main(int argc, char *argv[])
	try
{
	const Param p(argc, argv);
	SysInit();
	if (p.mode == "init") {
		Init();
	} else
	if (p.mode == "enc") {
		Enc(p.iv);
	} else
	if (p.mode == "sum") {
		Sum();
	} else
	if (p.mode == "dec") {
		Dec();
	} else
	{
		printf("bad mode=%s\n", p.mode.c_str());
		return 1;
	}
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
}
