#include <xbyak_aarch64/xbyak_aarch64.h>
#include <memory.h>

uint32_t f2u(float f)
{
	uint32_t u;
	memcpy(&u, &f, sizeof(u));
	return u;
}

using namespace Xbyak_aarch64;

struct Code : CodeGenerator {
	Code(int mode)
	{
		const auto& dst = x0;
		const auto& src = x1;
		ptrue(p0.s);
		switch (mode) {
		case 0:
			puts("ld1w");
			ld1w(z0.s, p0, ptr(src, 1));
			st1w(z0.s, p0, ptr(dst));
			break;
		case 1:
			puts("ldr");
			ldr(z0, ptr(src, 1));
			str(z0, ptr(dst));
			break;
		case 2:
			puts("ld1rw"); // broadcast
			ld1rw(z0.s, p0, ptr(src, 64));
			str(z0, ptr(dst));
			break;
		default:
			fprintf(stderr, "bad mode=%d\n", mode);
			exit(1);
		}
		ret();
	}
};


int main()
	try
{
	const size_t N = 16;
	float x[N * 2], y[N];
	for (int mode = 0; mode < 4; mode++) {
		for (size_t i = 0; i < N; i++) {
			x[i] = 0;
			x[i + N] = -(i * 0.123) + 0.4;
			y[i] = 0;
		}
		Code c(mode);
		auto f = c.getCode<void (*)(float *, const float *)>();
		c.ready();
		f(y, x);
		for (size_t i = 0; i < N; i++) {
			float a = x[N + i];
			float b = y[i];
			if (mode == 2) a = x[N]; // broadcast
			printf("%zd x=%f y=%f(%c)\n", i, a, b, f2u(a) == f2u(b) ? 'o' : 'x');
		}
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
