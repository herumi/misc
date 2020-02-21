/**
	@author herumi
	@note modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
	pow(2.0f, x) for x in [1, 2]
*/
#include <xbyak/xbyak_util.h>
#include <cmath>

struct Data {
	static const size_t TaylerN = 5;
	float log2div4; // log(2)/4
	uint32_t expCoeff[TaylerN]; // near to 1/(i + 1)!
	Data()
		: log2div4(std::log(2.0f) / 4.0f)
	{
		const uint32_t tbl[TaylerN] = {
			0x3f800000,
			0x3effff12,
			0x3e2aaa56,
			0x3d2b89cc,
			0x3c091331,
		};
		for (size_t i = 0; i < TaylerN; i++) {
			expCoeff[i] = tbl[i];
		}
	}
} g_data;

typedef void (*VecFunc)(float *dst, const float *src, size_t n);

using namespace Xbyak;

/*
	pow2_v(float *dst, const float *src, size_t n)
	{
		assert(1 <= src[i] && src[i] <= 2);
		for (size_t i = 0; i < n; i++) dst[i] = std::pow(2.0f, src[i]);
	}
	2^x = exp(x log(2)) = exp(x log(2)/4)^4
*/
struct Code : public Xbyak::CodeGenerator {
	Code() : Xbyak::CodeGenerator() {
		Xbyak::util::Cpu cpu;
		if (!cpu.has(Xbyak::util::Cpu::tAVX512F)) {
			fprintf(stderr, "AVX-512 is not supported\n");
			exit(1);
		}
		const int keepRegN = 3;
		util::StackFrame sf(this, 3, util::UseRCX, 64 * keepRegN);
		const Reg64& dst = sf.p[0];
		const Reg64& src = sf.p[1];
		const Reg64& n = sf.p[2];

		// prolog
#ifdef XBYAK64_WIN
		vmovups(ptr[rsp + 64 * 0], zm6);
		vmovups(ptr[rsp + 64 * 1], zm7);
#endif
		vmovups(ptr[rsp + 64 * 2], Zmm(8));

		// setup constant
		const Zmm& log2div4 = zmm3;
		const Zmm expCoeff[] = { zmm4, zmm5, zmm6, zmm7, zmm8 };

		mov(rax, size_t(&g_data));
		vpbroadcastd(log2div4, ptr[rax + offsetof(Data, log2div4)]);
		for (size_t i = 0; i < Data::TaylerN; i++) {
			vpbroadcastd(expCoeff[i], ptr[rax + offsetof(Data, expCoeff[0]) + sizeof(float) * i]);
		}

	Label lp = L();
		vmovups(zm0, ptr[src]);
		add(src, 64);
		vmulps(zm0, log2div4); // y = x * log(2)/4
		// compute exp(y)
		vmovaps(zm1, expCoeff[4]);
		vfmadd213ps(zm1, zm0, expCoeff[3]);
		vfmadd213ps(zm1, zm0, expCoeff[2]);
		vfmadd213ps(zm1, zm0, expCoeff[1]);
		vfmadd213ps(zm1, zm0, expCoeff[0]);
		vfmadd213ps(zm1, zm0, expCoeff[0]);
		// exp(y)^2
		vmulps(zm0, zm1, zm1);
		// exp(y)^4 = exp(4y) = exp(x log(2)) = 2^x
		vmulps(zm0, zm0);
		vmovups(ptr[dst], zm0);
		add(dst, 64);
		sub(n, 16);
		jnz(lp);

#ifdef XBYAK64_WIN
		vmovups(zm6, ptr[rsp + 64 * 0]);
		vmovups(zm7, ptr[rsp + 64 * 1]);
#endif
		vmovups(Zmm(8), ptr[rsp + 64 * 2]);
	}
} g_code;

VecFunc pow2_v;

float pow2(float x)
{
	float y[16] = {x};
	pow2_v(y, y, 16);
	return *y;
}

int main()
{
	pow2_v = g_code.getCode<VecFunc>();
	float maxe = 0;
	for (float x = 1; x <= 2; x += 1e-6) {
		float y1 = pow(2.0f, x);
		float y2 = pow2(x);
		float e = std::abs(y1 - y2) / y1;
		if (e > maxe) {
			maxe = e;
		}
//		printf("x=%e, y1=%e, y2=%e\n", x, y1, y2);
	}
	printf("maxe=%e\n", maxe);
	const size_t n = 32768;
	static float src[n];
	static float dst[n];
	for (size_t i = 0; i < n; i++) {
		src[i] = 1 + i / float(n);
	}
	const size_t C = 100000;
	util::Clock clk;
	for (size_t i = 0; i < C; i++) {
		clk.begin();
		pow2_v(dst, src, n);
		clk.end();
	}
	printf("clk/element=%4.2f clk\n", clk.getClock() / double(C) / n);
}
