#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <memory.h>

void change(double * /* a */, int k1, int j1)
{
#if 1
	printf("__m128d x = _mm_load_pd(a + %d);\n", j1);
	printf("__m128d y = _mm_load_pd(a + %d);\n", k1);
	printf("_mm_store_pd(a + %d, y);\n", j1);
	printf("_mm_store_pd(a + %d, x);\n", k1);
#else
	__m128d x = _mm_load_pd(a + j1);
	__m128d y = _mm_load_pd(a + k1);
	_mm_store_pd(a + j1, y);
	_mm_store_pd(a + k1, x);
#endif
}

void bitrv2_C(int n, int *ip, double *a)
{
	int j, j1, k, k1, l, m, m2;

	ip[0] = 0;
	l = n;
	m = 1;
	while ((m << 3) < l) {
		l >>= 1;
		for (j = 0; j < m; j++) {
			ip[m + j] = ip[j] + l;
		}
		m <<= 1;
	}
	m2 = 2 * m;
	if ((m << 3) == l) {
		for (k = 0; k < m; k++) {
			for (j = 0; j < k; j++) {
				j1 = 2 * j + ip[k];
				k1 = 2 * k + ip[j];
				change(a, j1, k1);
				j1 += m2;
				k1 += 2 * m2;
				change(a, j1, k1);
				j1 += m2;
				k1 -= m2;
				change(a, j1, k1);
				j1 += m2;
				k1 += 2 * m2;
				change(a, j1, k1);
			}
			j1 = 2 * k + m2 + ip[k];
			k1 = j1 + m2;
			change(a, j1, k1);
		}
	} else {
		for (k = 1; k < m; k++) {
			for (j = 0; j < k; j++) {
				j1 = 2 * j + ip[k];
				k1 = 2 * k + ip[j];
				change(a, j1, k1);
				j1 += m2;
				k1 += m2;
				change(a, j1, k1);
			}
		}
	}
}

struct Code : Xbyak::CodeGenerator {
	Code(int n)
	{
		int ip[512];
		gen_bitrv2(n, ip);
	}
	void change(const Xbyak::Reg32e& a, int k1, int j1)
	{
		movapd(xm0, ptr [a + j1 * 8]);
		movapd(xm1, ptr [a + k1 * 8]);
		movapd(ptr [a + j1 * 8], xm1);
		movapd(ptr [a + k1 * 8], xm0);
	}

	// void bitrv2(double *x);
	void gen_bitrv2(int n, int *ip)
	{
		using namespace Xbyak;
#if defined(XBYAK32)
		const Reg32& a = eax;
		mov(eax, ptr [esp + 4]);
#elif defined(XBYAK64_WIN)
		const Reg64& a = r8;
#else
		const Reg64& a = rdi;
#endif
		int j, j1, k, k1, l, m, m2;

		ip[0] = 0;
		l = n;
		m = 1;
		while ((m << 3) < l) {
			l >>= 1;
			for (j = 0; j < m; j++) {
				ip[m + j] = ip[j] + l;
			}
			m <<= 1;
		}
		m2 = 2 * m;
		if ((m << 3) == l) {
			for (k = 0; k < m; k++) {
				for (j = 0; j < k; j++) {
					j1 = 2 * j + ip[k];
					k1 = 2 * k + ip[j];
					change(a, j1, k1);
					j1 += m2;
					k1 += 2 * m2;
					change(a, j1, k1);
					j1 += m2;
					k1 -= m2;
					change(a, j1, k1);
					j1 += m2;
					k1 += 2 * m2;
					change(a, j1, k1);
				}
				j1 = 2 * k + m2 + ip[k];
				k1 = j1 + m2;
				change(a, j1, k1);
			}
		} else {
			for (k = 1; k < m; k++) {
				for (j = 0; j < k; j++) {
					j1 = 2 * j + ip[k];
					k1 = 2 * k + ip[j];
					change(a, j1, k1);
					j1 += m2;
					k1 += m2;
					change(a, j1, k1);
				}
			}
		}
		ret();
	}
};

int main()
	try
{
	MIE_ALIGN(16) double a[4096];
	{
		int ip[512];
		bitrv2_C(64, ip, a);
	}

	Code c(64);
	memset(a, 0, sizeof(a));
	void (*f)(double *) = (void (*)(double*))c.getCode();
	printf("ptr=%p\n", a);
	f(a);
} catch (std::exception& e) {
	printf("ERR:%s\n", e.what());
	return 1;
}
