/*
	sample of bf16
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <xbyak/xbyak_util.h>
#include <math.h>

union fi {
	float f;
	uint32_t u;
};

typedef uint16_t bfloat16;

float u2f(uint32_t u)
{
	fi fi;
	fi.u = u;
	return fi.f;
}

uint32_t f2u(float f)
{
	fi fi;
	fi.f = f;
	return fi.u;
}

bfloat16 float_to_bfloat16(float f)
{
	// ignore denormal and infinity
	uint32_t u = f2u(f);
	uint32_t rounding = 0x7fff + ((u >> 16) & 1);
	u += rounding;
	return bfloat16(u >> 16);
}

float bfloat16_to_float(bfloat16 f)
{
	return u2f(f << 16);
}

struct Code : Xbyak::CodeGenerator {
	Code()
	{
	}
	void gen_vdpbf16ps()
	{
		align(16);
		Xbyak::util::StackFrame sf(this, 3);
		vmovups(zm0, ptr[sf.p[0]]);
		vmovups(zm1, ptr[sf.p[1]]);
		vmovups(zm2, ptr[sf.p[2]]);
		vdpbf16ps(zm0, zm1, zm2);
		vmovups(ptr[sf.p[0]], zm0);
		ret();
	}
	void gen_vcvtne2ps2bf16()
	{
		align(16);
		Xbyak::util::StackFrame sf(this, 3);
		vmovups(zm0, ptr[sf.p[0]]);
		vmovups(zm1, ptr[sf.p[1]]);
		vmovups(zm2, ptr[sf.p[2]]);
		vcvtne2ps2bf16(zm0, zm1, zm2);
		vmovups(ptr[sf.p[0]], zm0);
		ret();
	}
};

void vdpbf16psC(float *dst, const bfloat16 *src1, const bfloat16 *src2)
{
	for (int i = 0; i < 16; i++) {
		dst[i] += bfloat16_to_float(src1[i*2+0]) * bfloat16_to_float(src2[i*2+0]);
		dst[i] += bfloat16_to_float(src1[i*2+1]) * bfloat16_to_float(src2[i*2+1]);
	}
}

void vcvtne2ps2bf16C(bfloat16 *dst, const float *src1, const float *src2)
{
	for (int i = 0; i < 16; i++) {
		dst[i] = float_to_bfloat16(src1[i]);
		dst[i+16] = float_to_bfloat16(src2[i]);
	}
}

char diff(float x, float y)
{
	return fabs(x - y) < 1e-5f ? 'o' : 'x';
}

int main()
	try
{
	Xbyak::util::Cpu cpu;
	if (!cpu.has(Xbyak::util::Cpu::tAVX512_BF16)) {
		printf("AVX512_BF16 is not supported\n");
		return 1;
	}
	Code c;
	auto vdpbf16ps = c.getCurr<void (*)(void*, const void*, const void*)>();
	c.gen_vdpbf16ps();
	auto vcvtne2ps2bf16 = c.getCurr<void (*)(void*, const void*, const void*)>();
	c.gen_vcvtne2ps2bf16();
	bfloat16 src1[32];
	bfloat16 src2[32];
	float dst1[16];
	float dst2[16];
	for (int i = 0; i < 32; i++) {
		src1[i] = float_to_bfloat16(i * 0.1f + 1.0f);
		src2[i] = float_to_bfloat16(i * 0.3f - 4.0f);
	}
	for (int i = 0; i < 16; i++) {
		float x = i * 0.3f + 0.2f;
		dst1[i] = dst2[i] = x;
	}
	vdpbf16ps(dst1, src1, src2);
	vdpbf16psC(dst2, src1, src2);
	for (int i = 0; i < 16; i++) {
		printf("i=% 2d %f %f %c\n", i, dst1[i], dst2[i], diff(dst1[i], dst2[i]));
	}
	vcvtne2ps2bf16(src1, dst1, dst2);
	vcvtne2ps2bf16C(src2, dst1, dst2);
	for (int i = 0; i < 32; i++) {
		printf("i=% 2d %04x %04x %d\n", i, src1[i], src2[i], abs(src1[i] - src2[i]));
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}
