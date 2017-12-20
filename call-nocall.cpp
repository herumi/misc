/*
	verify the report
	http://eli.thegreenplace.net/2013/12/03/intel-i7-loop-performance-anomaly/

	                i7 4770 i7-3930K   i7-2600
	mov + add + mov
	loopCall           6.00     5.16      6.31
	loopNoCall         6.49     5.76      6.79

	read-modify-write
	loopCall           6.00     5.06      5.41
	loopNoCall         6.02     6.16      6.74
*/
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

const int  N = 100 * 1000 * 1000;
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
#if 0
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
}
