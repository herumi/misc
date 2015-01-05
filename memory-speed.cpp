#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <cybozu/option.hpp>
#include <cybozu/array.hpp>

template<class T>
void f(int& c, const T& v, size_t pos)
{
	c += v[pos];
}
void test(size_t shiftSize, bool random)
{
	const size_t dataSize = (size_t(1) << shiftSize) / 8;
	printf("dataSize=%.2fMiB\n", dataSize  * 8 / 1024. / 1024.);
	cybozu::AlignedArray<uint64_t> v;
	v.resize(dataSize);
	int c = 0;
	if (random) {
		cybozu::XorShift rg;
		CYBOZU_BENCH("random", f, c, v, rg.get64() & (dataSize - 1));
	} else {
		int pos = 0;
		CYBOZU_BENCH("seq", f, c, v, pos++ & (dataSize - 1));
	}
	printf("c=%d\n", c);
}
int main(int argc, char *argv[])
{
	size_t shiftSize;
	bool random;
	cybozu::Option opt;
	opt.appendOpt(&shiftSize, 10, "s", "data size : (1<<size) [byte]");
	opt.appendBoolOpt(&random, "r", ": random access");
	if (!opt.parse(argc, argv)) {
		opt.usage();
		return 1;
	}
	test(shiftSize, random);
}
