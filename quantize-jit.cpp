#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

using namespace Xbyak;
using namespace Xbyak::util;

const size_t N = 64;

typedef int (*DivFunc)(int);
typedef void (*QuantizerFunc)(int *tbl);

//#define DIV_C
#define DIV_ROUND

#ifdef DIV_C
int divRef(int x, int d)
{
	return x / d;
}
#endif
#ifdef DIV_ROUND
int divRef(int x, int d)
{
	if (x > 0) {
		return (x + (d/2)) / d;
	} else {
		return (x - (d/2)) / d;
	}
}
#endif

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
	int scale;
	if (q < 50) {
		scale = 5000 / q;
	} else {
		scale = 200 - 2 * q;
	}
	for (size_t i = 0; i < N; i++) {
		int v = (orgTbl[i] * scale + 50) / 100;
		if (v == 0) v = 1;
		if (v >= 255) v = 255;
		qTbl[i] = uint8_t(v);
	}
}

bool is2power(int d)
{
	return d && (d & (d - 1)) == 0;
}

struct Quantize : CodeGenerator {
	static const int a = 19;
	int c_;
	int d2_;
	int udiv(int x)
	{
		int mask = x >> 31;
		x = (x ^ mask) - mask;
		int y = ((x + d2_) * c_) >> a;
		y = (y ^ mask) - mask;
		return y;
	}
	explicit Quantize(const uint8_t *qTbl)
		: CodeGenerator(8192, Xbyak::DontSetProtectRWE)
	{
		StackFrame sf(this, 1, UseRDX);
		const Reg64& src = sf.p[0];
		for (size_t i = 0; i < N; i++) {
			const uint8_t d = qTbl[i];
			if (d == 1) continue;
			mov(eax, ptr[src + i * 4]);
			rawDiv(d);
			mov(ptr[src + i * 4], eax);
		}
	}
	explicit Quantize(int q)
		: CodeGenerator(4096, Xbyak::DontSetProtectRWE)
	{
		StackFrame sf(this, 1, UseRDX);
		const Reg64& x = sf.p[0];
		mov(eax, x.cvt32());
		rawDiv(q);
	}

	// input: eax
	// output: eax / d
	// destroy: edx
	void rawDiv(int d)
	{
		const int c = ((1 << a) + d - 1) / d;
		c_ = c;
		d2_ = d/2;
		mov(edx, eax);
		sar(edx, 31); // edx = mask = (eax < 0) ? 0xffffffff : 0
		xor_(eax, edx);
		sub(eax, edx);
#ifdef DIV_ROUND
		add(eax, d/2);
#endif
		imul(eax, eax, c);
		shr(eax, a);
		xor_(eax, edx);
		sub(eax, edx);
	}
};

void test(const int *src, const uint8_t *qTbl, QuantizerFunc f)
{
	puts("test");
	int dst[N];
	memcpy(dst, src, sizeof(dst));
	f(dst);
	for (size_t i = 0; i < N; i++) {
		int e = divRef(src[i], qTbl[i]);
		if (dst[i] != e) {
			printf("ERR i=%zd src[i]/qTbl[i]=%d/%d=%d dst[i]=%d\n", i, src[i], qTbl[i], e, dst[i]);
		}
	}
	puts("test ok");
}

void testDiv(int d)
{
	Quantize quant(d);
	const auto f = quant.getCode<DivFunc>();
	quant.setProtectModeRE();
	for (int i = -2048; i <= 2048; i++) {
		int e = divRef(i, d);
		int e2 = quant.udiv(i);
		if (e != e2) {
			printf("ERR2 div %d/%d=%d v=%d\n", i, d, e, e2);
		}
		int v = f(i);
		if (v != e) {
			printf("ERR div %d/%d=%d v=%d\n", i, d, e, v);
		}
	}
}

void testAll()
{
	puts("testAll");
	for (int d = 1; d <= 255; d++) {
		testDiv(d);
	}
	puts("testAll ok");
}

void saveFile(const char *name, const uint8_t *buf, size_t size)
{
	FILE *fp = fopen(name, "wb");
	if (fp) {
		fwrite(buf, 1, size, fp);
		fclose(fp);
	}
}

int main(int argc, char *argv[])
	try
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
	puts("saved");

	int src[N];
	puts("src");
	for (size_t i = 0; i < N; i++) {
		src[i] = (rand() % 4097) - 2048;
		printf("%d ", src[i]);
	}
	puts("");
	test(src, qTbl, f);
	testAll();
} catch (std::exception& e) {
	printf("exception %s\n", e.what());
}
