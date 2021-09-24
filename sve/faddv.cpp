#include <xbyak_aarch64/xbyak_aarch64.h>

using namespace Xbyak_aarch64;

struct Code : CodeGenerator {
	Code()
	{
		ptrue(p0.s);
		ld1w(z0.s, p0, ptr(x0));
		faddv(s0, p0, z0.s);
		ret();
	}
};

int main()
	try
{
	const int N = 16;
	float d[N];
	for (int i = 0; i < N; i++) {
		d[i] = (i + 1) / 10.0f;
	}
	Code c;
	auto f = c.getCode<float (*)(const float *)>();
	c.ready();
	float sum1 = f(d);
	float sum2 = 0;
	for (int i = 0; i < N; i++) {
		sum2 += d[i];
	}
	printf("sum1=%f sum2=%f\n", sum1, sum2);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
