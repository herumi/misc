#include <xbyak/xbyak_util.h>

// void perm(float dst[16], const float src[16], const uint32_t idx[16]);
struct Code : Xbyak::CodeGenerator {
	Code()
	{
		Xbyak::util::StackFrame sf(this, 3);
		vmovups(zm1, ptr[sf.p[1]]);
		vmovups(zm2, ptr[sf.p[2]]);
		vpermd(zm0, zm1, zm2);
		vmovups(ptr[sf.p[0]], zm0);
	}
};

void put(const char *msg, const float x[16])
{
	printf("%s", msg);
	for (int i = 0; i < 16; i++) {
		printf(" %.2f", x[i]);
	}
	printf("\n");
}

int main()
{
	Code c;
	auto perm = c.getCode<void (*)(float[16], const uint32_t[16], const float[16])>();
	float in[16], out[16];
	uint32_t idx[16];
	for (int i = 0; i < 16; i++) {
		in[i] = i * 0.1f;
	}
	for (int i = 0; i < 16; i++) {
		idx[i] = 15 - i;
	}
	put("in ", in);
	perm(out, idx, in);
	put("out", out);
}
