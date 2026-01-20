#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

using namespace Xbyak;
using namespace Xbyak::util;

const size_t N = 64;

typedef uint8_t (*DivFunc)(uint8_t);
typedef void (*QuantizerFunc)(uint8_t *tbl);

void setTable(uint8_t *qTbl, int q)
{
	static const uint32_t orgTbl[] = {
		16, 11, 10, 16, 24, 40, 51, 61,
		12, 12, 14, 19, 26, 58, 60, 55,
		14, 13, 16, 24, 40, 57, 69, 56,
		14, 17, 22, 29, 51, 87, 80, 62,
		18, 22, 37, 56, 68, 109, 103, 77,
		24, 35, 55, 64, 81, 104, 113, 92,
		49, 64, 78, 87, 103, 121, 120, 101,
		72, 92, 95, 98, 112, 100, 103, 99
	};
	for (size_t i = 0; i < N; i++) {
		int v = orgTbl[i] / q;
		if (v == 0) v = 1;
		qTbl[i] = uint8_t(v);
	}
}

bool is2power(uint8_t d)
{
	return d && (d & (d - 1)) == 0;
}

struct Quantize : CodeGenerator {
	explicit Quantize(const uint8_t *qTbl)
		: CodeGenerator(8192, Xbyak::DontSetProtectRWE)
	{
		StackFrame sf(this, 1);
		const Reg64& src = sf.p[0];
		for (size_t i = 0; i < N; i++) {
			const uint8_t d = qTbl[i];
			if (d == 1) continue;
			movzx(eax, byte[src + i]);
			rawDiv(d);
			mov(ptr[src + i], al);
		}
	}
	explicit Quantize(uint8_t q)
		: CodeGenerator(4096, Xbyak::DontSetProtectRWE)
	{
		StackFrame sf(this, 1);
		const Reg64& x = sf.p[0];
		movzx(eax, x.cvt8());
		rawDiv(q);
	}

	// input: eax
	// output: eax / d
	// assume: eax < 256, 0 < d < 256
	void rawDiv(uint8_t d)
	{
		assert(d > 0);
		if (is2power(d)) {
			int shift = -1;
			while (d) {
				shift++;
				d >>= 1;
			}
			if (shift > 0) shr(eax, shift);
		} else if (d > 128) {
			cmp(eax, d);
			setge(al); // eax >= d ? 1 : 0
		} else {
			uint8_t shift = 15;
			uint32_t c = ((1u << shift) + d - 1) / d;
			imul(eax, eax, c);
			shr(eax, shift);
		}
	}
};

void test(const uint8_t *src, const uint8_t *qTbl, QuantizerFunc f)
{
	uint8_t dst[N];
	memcpy(dst, src, N);
	f(dst);
	for (size_t i = 0; i < N; i++) {
		int e = src[i] / qTbl[i];
		if (dst[i] != e) {
			printf("ERR i=%zd src[i]/qTbl[i]=%d/%d=%d dst[i]=%d\n", i, src[i], qTbl[i], e, dst[i]);
		}
	}
}

void testDiv(uint8_t d)
{
	Quantize quant(d);
	const auto f = quant.getCode<DivFunc>();
	quant.setProtectModeRE();
	for (int i = 0; i < 256; i++) {
		int e = i / d;
		int v = f(uint8_t(i));
		if (v != e) {
			printf("ERR div %d/%d=%d v=%d\n", i, d, e, v);
		}
	}
}

void testAll()
{
	for (int d = 1; d <= 255; d++) {
		testDiv(uint8_t(d));
	}
}

void saveFile(const char *name, const uint8_t *ptr, size_t size)
{
	FILE *fp = fopen(name, "wb");
	if (fp) {
		fwrite(ptr, 1, size, fp);
		fclose(fp);
	}
}

int main(int argc, char *argv[])
{
	const int q = argc > 1 ? atoi(argv[1]) : 50;
	printf("q=%d\n", q);
	uint8_t qTbl[N];
	setTable(qTbl, q);
	for (size_t i = 0; i < N; i++) {
		printf("%d ", qTbl[i]);
		if ((i % 16) == 15) putchar('\n');
	}

	Quantize quant(qTbl);
	quant.setProtectModeRE();
	const auto f = quant.getCode<QuantizerFunc>();
	saveFile("bin", quant.getCode(), quant.getSize());

	uint8_t src[N];
	for (size_t i = 0; i < N; i++) {
		src[i] = uint8_t(rand());
	}
	test(src, qTbl, f);
	puts("ok");
	testAll();
	puts("all div ok");
}
