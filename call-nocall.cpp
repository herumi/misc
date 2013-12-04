/*
	verify the report
	http://eli.thegreenplace.net/2013/12/03/intel-i7-loop-performance-anomaly/
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
		xor_(eax, eax);
		mov(rcx, (size_t)&counter);
	L("@@");
		if (doCall) call("f");
		mov(rdx, ptr [rcx]);
		add(rdx, rax);
		add(rax, 1);
		mov(ptr [rcx], rdx);
		cmp(rax, N);
		jnz("@b");
		ret();
		if (doCall) {
			align(16);
	L("f");
			ret();
		}
	}
} s_code;

void test(void (*f)())
{
	Xbyak::util::Clock clk;
	counter = 0;
	clk.begin();
	f();
	clk.end();
	printf("counter=%lld %f\n", (long long)counter, clk.getClock() / double(clk.getCount()));
}
int main()
{
	test(loopCall);
	test(loopNoCall);
}
