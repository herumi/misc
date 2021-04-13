#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak_aarch64;

struct Code : public CodeGenerator {
	Code()
	{
		ptrue(p0.s);
		ld1w(z0.s, p0/T_z, ptr(x1));

#if 1
		fcpy(z2.s, p0, 1.0);
		frecpe(z0.s, z0.s); // y = approx(1/x)
		movprfx(z0.s, p0, z2.s);
		fmls(z0.s, p0, z0.s, z1.s); // z0 = 1 - xy ; nam if x = inf
		fmad(z0.s, p0, z0.s, z0.s);
		fmad(z0.s, p0, z1.s, z1.s);
#else
		frecpe(z1.s, z0.s);
		frecps(z2.s, z0.s, z1.s);
		fmul(z1.s, z1.s, z2.s);

		frecps(z2.s, z0.s, z1.s);
		fmul(z0.s, z1.s, z2.s);
#endif

		st1w(z0.s, p0, ptr(x0));
		ret();
	}
};

int main()
	try
{
	Code c;
	c.ready();
	auto f = c.getCode<void (*)(float*, const float*)>();
	float xs[16];
	float ys[16];
	xs[0] = INFINITY;
	f(ys, xs);
	printf("%f %f\n", xs[0], ys[0]);
	for (float x = 0.1; x < 10; x += 0.8) {
		xs[0] = x;
		f(ys, xs);
		printf("%f %f\n", xs[0], ys[0]);
	}


	puts("top code");
	const uint32_t *pc = (const uint32_t*)f;
	printf("%08x\n", *pc);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}
