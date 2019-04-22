/*
cl /EHsc -I ../xbyak/ expf512.cpp /Zi /W4 /arch:AVX2 && ..\intel\sde -skx -- expf512.exe
*/
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <xbyak/xbyak.h>
#include <math.h>

using namespace Xbyak;
using namespace Xbyak::util;

static const size_t N = 12;
static uint32_t cvalTbl[N][16];

struct Code : CodeGenerator {

	void initTable()
	{
		const unsigned int cvals[N] = {
			0x3f800000,		// [0] 1.0f
			0x3f000000,		// [1] 0.5f
			0x3fb8aa3b,		// [2] log2ef = 1.44269502f
			0x3f317218,		// [3] ln2f =   0.69314718f
			0x0000007f,		// [4] 0x7f
			// exp(x) polynom
			0x3f800001,		// [5] p0 = 1.0000001f
			0x3efffe85,		// [6] p2 = 0.4999887f
			0x3e2aaa3e,		// [7] p3 = 0.16666505f
			0x3d2bb1b1,		// [8] p4 = 0.041917507f
			0x3c091ec1,		// [9] p5 = 0.008369149f
			0x42b0c0a5, //[10] max logf = 88.3762589f
			0xc1766666, //[11] min logf = -14.5f
		};
		for (size_t i = 0; i < N; i++) {
			for (size_t j = 0; j < 16; j++) {
				cvalTbl[i][j] = cvals[i];
			}
		}
	}
	Address table_val(int n) const
	{
		return ptr[rax + n * 16 * 4];
	}

	// exp512(float *out, const float *in);
	Code()
	{
		initTable();
		const int _cmp_nle_us = 6;

		const Zmm & vmm_src = zmm0;
		const Zmm & vmm_aux0 = zmm1;
		const Zmm & vmm_aux1 = zmm2;

		StackFrame sf(this, 2);
		const Reg64& dst = sf.p[0];
		const Reg64& src = sf.p[1];
#if 0

#ifdef XBYAK64_WIN
		const Reg64& dst = rcx;
		const Reg64& src = rdx;
#else
		const Reg64& dst = rdi;
		const Reg64& src = rsi;
#endif
#endif
		mov(rax, (size_t)cvalTbl);
		vmovups(vmm_src, ptr[src]);
//////////////////
		vminps(vmm_src, vmm_src, table_val(10));
		vmaxps(vmm_src, vmm_src, table_val(11));
		vmovups(vmm_aux0, vmm_src);

		//calculate exp(x)
		// fx = x * log2ef + 0.5
		vmulps(vmm_src, vmm_src, table_val(2));
		vaddps(vmm_src, vmm_src, table_val(1));

		// tmp = floorf(fx)
#if 1
		vrndscaleps(vmm_aux1, vmm_src, 1);
#else
		vcvtps2dq(vmm_aux1 | T_rd_sae, vmm_src);
		vcvtdq2ps(vmm_aux1, vmm_aux1);
#endif

#if 0
		const Zmm & vmm_aux3 = zmm4;
		Opmask k_mask = k1;
		vcmpps(k_mask, vmm_aux1, vmm_src, _cmp_nle_us);
		vmovups(vmm_aux3 | k_mask | T_z, table_val(0));
		vsubps(vmm_aux1, vmm_aux1, vmm_aux3);
#endif

		//keep fx for further computations
		vmovups(vmm_src, vmm_aux1);	//vmm_src = fx

		//x = x - fx * ln2
		vfnmadd231ps(vmm_aux0, vmm_aux1, table_val(3));

		// compute 2^n
		vcvtps2dq(vmm_aux1, vmm_src);
		vpaddd(vmm_aux1, vmm_aux1, table_val(4));
		vpslld(vmm_aux1, vmm_aux1, 23);	//Vmm(6) = 2^-fx

		// y = p5
		vmovups(vmm_src, table_val(9));
		// y = y * x + p4
		vfmadd213ps(vmm_src, vmm_aux0, table_val(8));
		// y = y * x + p3
		vfmadd213ps(vmm_src, vmm_aux0, table_val(7));
		// y = y * x + p2
		vfmadd213ps(vmm_src, vmm_aux0, table_val(6));
		// y = y * x + p1
		vfmadd213ps(vmm_src, vmm_aux0, table_val(0));
		// y = y * x + p0
		vfmadd213ps(vmm_src, vmm_aux0, table_val(5));	//exp(q)
		// y = y * 2^n
		vmulps(vmm_src, vmm_src, vmm_aux1);
/////////
		vmovups(ptr[dst], vmm_src);
		vzeroupper();
		ret();
	}
	void cvt(bool use_vrndscaleps)
	{
		align(16);
		StackFrame sf(this, 2);
		vmovups(zm0, ptr[sf.p[1]]);
		if (use_vrndscaleps) {
			vrndscaleps(zm0, zm0, 1);
		} else {
			vcvtps2dq(zm0 | T_rd_sae, zm0);
			vcvtdq2ps(zm0, zm0);
		}
		vmovups(ptr[sf.p[0]], zm0);
		ret();
	}
};

int main()
	try
{
	Code c;
	auto exp512f = c.getCode<void (*)(float *, const float*)>();
	float in[16] = { -1.3f, -0.5f, 0, 0.5f, 1.0f, 2.1f };
	float out1[16];
	float out2[16];
	for (int i = 0; i < 16; i++) {
		out1[i] = expf(in[i]);
	}
	exp512f(out2, in);
	for (int i = 0; i < 5; i++) {
		printf("in[%d] %7.5f %7.5f %7.5f %e\n", i, in[i], out1[i], out2[i], fabs(out1[i] - out2[i]));
	}

	auto cvt1 = c.getCurr<void (*)(float *, const float*)>();
	c.cvt(false);
	auto cvt2 = c.getCurr<void (*)(float *, const float*)>();
	c.cvt(true);
	float in2[16] = { 12.3f, -23.4f, 1.2345678909123e10f, -1.2345678909123e10f };
	float iout1[16], iout2[16];
	cvt1(iout1, in2);
	cvt2(iout2, in2);
	for (int i = 0; i < 4; i++) {
		printf("%f -> %f %f\n", in2[i], iout1[i], iout2[i]);
	}
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}

