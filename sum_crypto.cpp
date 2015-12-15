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
		std::string addList;
		opt.appendOpt(&addList, "1 5 3 2", "l", ": quoted list of int(eg. \"1 5 3 2\")");
		opt.appendHelp("h", ": put this message");
		opt.appendParam(&mode, "mode", ": init/enc/add/dec");
		if (!opt.parse(argc, argv)) {
			opt.usage();
			exit(1);
		}
		printf("mode=%s\n", mode.c_str());
		if (mode == "add") {
			printf("addList=%s\n", addList.c_str());
			std::istringstream iss(addList);
			int i;
			while (iss >> i) {
				iv.push_back(i);
			}
		}
	}
};

void SysInit()
{
	const mcl::EcParam& para = mcl::ecparam::secp192k1;
	Zn::setModulo(para.n);
	Fp::setModulo(para.p);
	Ec::setParam(para.a, para.b);
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
	puts("enc");
	Elgamal::PublicKey pub;
	Load(pub, pubFile);
	for (size_t i = 0; i < iv.size(); i++) {
		Elgamal::CipherText c;
		pub.enc(c, iv[i], rg);
		const std::string sheetName = GetSheetName(i);
		printf("make %s\n", sheetName.c_str());
		Save(sheetName, c);
	}
}

void Sum()
{
	puts("sum");
	Elgamal::PublicKey pub;
	Load(pub, pubFile);
	Elgamal::CipherText result;
	for (size_t i = 0; ; i++) {
		const std::string sheetName = GetSheetName(i);
		Elgamal::CipherText c;
		if (!Load(c, sheetName, false)) break;
		printf("add %s\n", sheetName.c_str());
		result.add(c);
	}
	printf("create result file : %s\n", resultFile.c_str());
	Save(resultFile, result);
}

void Dec()
{
	puts("dec");
	Elgamal::PrivateKey prv;
	Load(prv, prvFile);
	prv.setCache(0, 10000);
	Elgamal::CipherText c;
	Load(c, resultFile);
	int sum = prv.dec(c);
	std::cout << "result of sum " << sum << std::endl;
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
