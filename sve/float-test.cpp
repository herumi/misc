#include <xbyak_aarch64/xbyak_aarch64.h>
#include <functional>

using namespace Xbyak;

union fi {
	uint32_t u;
	float f;
};

float u2f(uint32_t x)
{
	fi fi;
	fi.u = x;
	return fi.f;
}

uint32_t f2u(float x)
{
	fi fi;
	fi.f = x;
	return fi.u;
}

const float g_c0 = 1.5;
const float g_c1 = 1.23456;
const float g_c2 = 3.45678;

struct Code : CodeGenerator {
	Xbyak::Label g_dataL;
	Code()
	{
		align(4096);
	L(g_dataL);
		printf("dataL=%p\n", g_dataL.getAddress());
		dw(f2u(g_c2));
		align(4096);
	}
	void generate(void (Code::*f)(const ZReg&, const ZReg&, const ZReg&))
	{
		const auto& pdst = x0;
		const auto& psrc1 = x1;
		const auto& psrc2 = x2;
		const auto& dst = z0;
		const auto& src1 = z1;
		const auto& src2 = z2;
		ptrue(p0.s);
		ld1w(src1.s, p0/T_z, ptr(psrc1));
		ld1w(src2.s, p0/T_z, ptr(psrc2));
		(this->*f)(dst, src1, src2);
		st1w(dst.s, p0/T_z, ptr(pdst));
		ret();
	}
	void faddW(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		fadd(dst.s, src1.s, src2.s);
	}
	void fsubW(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		fsub(dst.s, src1.s, src2.s);
	}
	void fmulW(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		fmul(dst.s, src1.s, src2.s);
	}
	void fdivW(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		movprfx(dst.s, p0, src1.s);
		fdiv(dst.s, p0, src2.s);
	}
	/*
		t = rcp(x)
		1/x = (2 - xt)t
	*/
	// destroy src2
	// err < 1e-5
	void invW(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		frecpe(src2.s, src1.s);
		frecps(dst.s, src1.s, src2.s);
		fmul(dst.s, dst.s, src2.s);
	}
	void frecpsW(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		frecps(dst.s, src1.s, src2.s);
	}
	void fmaxW(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		movprfx(dst.s, p0, src1.s);
		fmax(dst.s, p0, src2.s);
	}
	void fcpyW(const ZReg& dst, const ZReg& src1, const ZReg&)
	{
		fcpy(dst.s, p0, g_c0);
	}
	void cpyW(const ZReg& dst, const ZReg&, const ZReg&)
	{
		Xbyak::Label dataL, skipL;
		adr(x3, dataL);
		ldr(w3, ptr(x3));
		cpy(dst.s, p0, w3);
		b(skipL);
	L(dataL);
		printf("dataL=%p\n", dataL.getAddress());
		dw(f2u(g_c1));
	L(skipL);
	}
	void cpyWithAdrpW(const ZReg& dst, const ZReg&, const ZReg&)
	{
		Xbyak::Label skipL;
		b(skipL);
		align(4096);
	L(skipL);
		adrp(x3, g_dataL);
		ldr(w3, ptr(x3));
		cpy(dst.s, p0, w3);
	}
};

void test_ld1w_imm()
{
	puts("test_ld1w_imm");
	const int srcAdj = 2;
	const int dstAdj = 3;
	struct Code : Xbyak::CodeGenerator {
		Code()
		{
			// copy(float *dst, const float *src);
			ptrue(p0.s);
			ld1w(z0.s, p0, ptr(x1, srcAdj));
			st1w(z0.s, p0, ptr(x0, dstAdj));
			ret();
		}
	};

	float src[16];
	float dst[16];
	for (int i = 0; i < 16; i++) {
		src[i] = float(i);
	}
	Code c;
	c.ready();
	auto f = c.getCode<void (*)(float *, const float *)>();
	f(dst - dstAdj * 16, src - srcAdj * 16);
	for (int i = 0; i < 16; i++) {
		float s = float(i);
		float d = dst[i];
		if (s != d) {
			printf("err %d s=%f d=%f\n", i, s, d);
		}
	}
	puts("ok");
}

void test_fscale()
{
	puts("test_fscale");
	struct Code : Xbyak::CodeGenerator {
		Code()
		{
			// copy(float *dst, const float *src, const int *e); // dst = src * 2^e
			ptrue(p0.s);
			ld1w(z0.s, p0, ptr(x1));
			ld1w(z1.s, p0, ptr(x2));
			fscale(z0.s, p0, z1.s);
			st1w(z0.s, p0, ptr(x0));
			ret();
		}
	};
	int ex[16];
	float src[16];
	float dst[16];
	for (int i = 0; i < 16; i++) {
		ex[i] = i;
		src[i] = (i + 10) / 8.0f;
	}
	Code c;
	c.ready();
	auto f = c.getCode<void (*)(float *, const float *, const int *)>();
	f(dst, src, ex);
	for (int i = 0; i < 16; i++) {
		float s = src[i];
		float d = dst[i];
		int e = ex[i];
		printf("fscale %d d=%f %f*2^%d=%f\n", i, d, s, e, s * pow(2, e));
	}
}

const size_t N = 16;

template<class F>
void vec(const F& f, float *z, const float *x, const float *y)
{
	for (size_t i = 0; i < N; i++) {
		z[i] = f(x[i], y[i]);
	}
}

float faddC(float x, float y) { return x + y; }
float fsubC(float x, float y) { return x - y; }
float fmulC(float x, float y) { return x * y; }
float fdivC(float x, float y) { return x / y; }
float invC(float x, float) { return 1 / x; }
float frecpsC(float x, float y) { return 2 - x * y; }
float fmaxC(float x, float y) { return std::max(x, y); }
float fcpyC(float, float) { return g_c0; }
float cpyC(float, float) { return g_c1; }
float cpy2C(float, float) { return g_c2; }

void check(const float *x, const float *y, const float *z1, const float *z2)
{
	bool ok = true;
	for (size_t i = 0; i < N; i++) {
		if (z1[i] != z2[i]) {
			printf("ERR i=% 2zd x=%f y=%f z1=%f z2=%f\n", i, x[i], y[i], z1[i], z2[i]);
			ok = false;
		}
	}
	if (ok) puts("ok");
}

void check2(const float *x, const float *z1, const float *z2)
{
	bool ok = true;
	for (size_t i = 0; i < N; i++) {
		float d = fabs((z1[i] - z2[i]) / z1[i]);
		if (d > 1e-6) {
			printf("ERR i=% 2zd x=%e z1=%e z2=%e\n", i, x[i], z1[i], z2[i]);
			ok = false;
		}
	}
	if (ok) puts("ok");
}

int main()
	try
{
	float x[N], y[N], z1[N], z2[N];
	for (size_t i = 0; i < N; i++) {
		x[i] = i + 3;
		y[i] = i * 2;
		z1[i] = -99;
		z2[i] = z1[i];
	}
	Code c;
	auto faddA = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::faddW);

	auto fsubA = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::fsubW);

	auto fmulA = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::fmulW);

	auto fdivA = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::fdivW);

	auto invA = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::invW);

	auto frecpsA = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::frecpsW);

	auto fmaxA = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::fmaxW);

	auto fcpyA = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::fcpyW);

	auto cpyA = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::cpyW);

	auto cpy2A = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::cpyWithAdrpW);
	c.ready();

	puts("fadd");
	vec(faddC, z1, x, y);
	faddA(z2, x, y);
	check(x, y, z1, z2);

	puts("fsub");
	vec(fsubC, z1, x, y);
	fsubA(z2, x, y);
	check(x, y, z1, z2);

	puts("fmul");
	vec(fmulC, z1, x, y);
	fmulA(z2, x, y);
	check(x, y, z1, z2);

	puts("fdiv");
	vec(fdivC, z1, x, y);
	fdivA(z2, x, y);
	check(x, y, z1, z2);

	puts("inv");
	vec(invC, z1, x, y); // z1 = 1/x
	invA(z2, x, y);
	check2(x, z1, z2);

	puts("frecps");
	vec(frecpsC, z1, x, y);
	frecpsA(z2, x, y);
	check(x, y, z1, z2);

	puts("fmax");
	vec(fmaxC, z1, x, y);
	fmaxA(z2, x, y);
	check(x, y, z1, z2);

	puts("fcpy"); // copy g_c0
	vec(fcpyC, z1, x, y);
	fcpyA(z2, x, y);
	check(x, y, z1, z2);

	puts("cpy"); // copy g_c1
	printf("g_c1=%f(%08x)\n", g_c1, f2u(g_c1));
	vec(cpyC, z1, x, y);
	cpyA(z2, x, y);
	check(x, y, z1, z2);

	puts("cpy with adrp"); // copy g_c2
	printf("g_c1=%f(%08x)\n", g_c2, f2u(g_c2));
	vec(cpy2C, z1, x, y);
	cpy2A(z2, x, y);
	check(x, y, z1, z2);
	test_ld1w_imm();
	test_fscale();
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
