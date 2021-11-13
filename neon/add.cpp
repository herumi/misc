#include <xbyak_aarch64/xbyak_aarch64.h>
#include <arm_neon.h>

using namespace Xbyak_aarch64;

class Generator : public CodeGenerator {
public:
	Generator()
	{
		fadd(v0.s, v0.s, v0.s);
		ret();
	}
};

int main() {
	Generator gen;
	gen.ready();
	auto f = gen.getCode<float32x4_t (*)(float32x4_t)>();
	float32x4_t a = { -3.4, 4.2, 1.5, 5.3 };
	a = f(a);
	float v[4];
	memcpy(v, &a, sizeof(v));
	printf("%f %f %f %f\n", v[0], v[1], v[2], v[3]);
}
