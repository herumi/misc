#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak;

struct Code : CodeGenerator {
	void generate()
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
		maxGen(dst, src1, src2);
		st1w(dst.s, p0/T_z, ptr(pdst));
		ret();
	}
	template<class T>
	void maxGen(const T&dst, const T& src1, const T& src2)
	{
		fadd(dst.s, src1.s, src2.s);
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

float add(float x, float y) { return x + y; }

int main()
	try
{
	Code c;
	auto f = c.getCurr<void (*)(float *, const float *, const float *)>();
	c.generate();
	c.ready();
	float x[N], y[N], z1[N], z2[N];
	for (size_t i = 0; i < N; i++) {
		x[i] = i;
		y[i] = i * 2;
		z1[i] = -99;
		z2[i] = z1[i];
	}
	vec(add, z1, x, y);
	f(z2, x, y);
	for (size_t i = 0; i < N; i++) {
		printf("x=%f y=%f z1=%f z2=%f %c\n", x[i], y[i], z1[i], z2[i], (z1[i] == z2[i]) ? 'o' : 'x');
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
