#include <xbyak_aarch64/xbyak_aarch64.h>
#include <functional>

using namespace Xbyak;

struct Code : CodeGenerator {
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
	void Wfadd(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		fadd(dst.s, src1.s, src2.s);
	}
	void Wfsub(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		fsub(dst.s, src1.s, src2.s);
	}
	void Wfmax(const ZReg& dst, const ZReg& src1, const ZReg& src2)
	{
		movprfx(dst.s, p0, src1.s);
		fmax(dst.s, p0, src2.s);
	}
};

const size_t N = 16;

template<class F>
void vec(const F& f, float *z, const float *x, const float *y)
{
	for (size_t i = 0; i < N; i++) {
		z[i] = f(x[i], y[i]);
	}
}

float addOne(float x, float y) { return x + y; }
float subOne(float x, float y) { return x - y; }
float maxOne(float x, float y) { return std::max(x, y); }

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
	auto Padd = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::Wfadd);
	auto Psub = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::Wfsub);
	auto Pmax = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate(&Code::Wfmax);
	c.ready();
	puts("add");
	vec(addOne, z1, x, y);
	Padd(z2, x, y);
	check(x, y, z1, z2);
	puts("sub");
	vec(subOne, z1, x, y);
	Psub(z2, x, y);
	check(x, y, z1, z2);
	puts("max");
	vec(maxOne, z1, x, y);
	Pmax(z2, x, y);
	check(x, y, z1, z2);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
