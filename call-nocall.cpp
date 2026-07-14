/*
	verify the report
	http://eli.thegreenplace.net/2013/12/03/intel-i7-loop-performance-anomaly/

	                i7 4770 i7-3930K   i7-2600  i7-7700 Xeon-Platinum-8280
	mov + add + mov
	loopCall           6.00     5.16      6.31    4.02   3.19
	loopNoCall         6.49     5.76      6.79    4.79   3.66

	read-modify-write
	loopCall           6.00     5.06      5.41    4.04
	loopNoCall         6.02     6.16      6.74    4.73

	2026-07-14 summary, read-modify-write (avg of runs; clk = rdtsc tick,
	not core cycle; old CPUs copied from the table above)
	              i7-2600  i7-3930K   i7-4770   i7-7700 Xeon-8280 i7-1165G7  i7-1255U i9-13900H  w9-3495X
	                  SNB     SNB-E   Haswell  KabyLake  CascadeL    TigerL    AlderL   RaptorL SapphireR
	loopCall         5.41      5.06      6.00      4.04      3.16      3.82      4.50      4.22      3.16
	loopNoCall       6.74      6.16      6.02      4.73      3.68      4.57      4.42      4.15      3.02

	the anomaly (loopCall faster) reproduces from Sandy Bridge through
	Willow Cove (Tiger Lake) and is gone since Golden Cove (Alder Lake).
*/
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

const int  N = 100 * 1000 * 1000 * 10;
int counter = 0;

void (*loopCall)();
void (*loopNoCall)();

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		align(16);
		loopCall = getCurr<void (*)()>();
		gen(true);

		align(16);
		loopNoCall = getCurr<void (*)()>();
		gen(false);
	}
	void gen(bool doCall)
	{
		mov(rax, N);
		mov(rcx, (size_t)&counter);
		Xbyak::Label Loop, JustRet;
	L(Loop);
		if (doCall) call(JustRet);
#if 1
		add(ptr [rcx], rax);
#else
		mov(rdx, ptr [rcx]);
		add(rdx, rax);
		mov(ptr [rcx], rdx);
#endif
		sub(rax, 1);
		jnz(Loop);
		ret();
		if (doCall) {
			align(16);
	L(JustRet);
			ret();
		}
	}
} s_code;

void test(const char *msg, void (*f)())
{
	Xbyak::util::Clock clk;
	counter = 0;
	clk.begin();
	f();
	clk.end();
	printf("%s %.2f clk/loop counter=%lld\n", msg, clk.getClock() / double(clk.getCount() * N), (long long)counter);
}
int main()
{
	test("w/call ", loopCall);
	test("wo/call", loopNoCall);
	test("w/call ", loopCall);
	test("wo/call", loopNoCall);
}
