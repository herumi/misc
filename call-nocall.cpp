/*
	verify the report
	http://eli.thegreenplace.net/2013/12/03/intel-i7-loop-performance-anomaly/
	https://news.ycombinator.com/item?id=15935283

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
/*
// Xeon 8280
misc% sudo sh call-nocall-perf.sh 0
=== call ===
w/call  3.15 clk/loop counter=-243309312
w/call  3.14 clk/loop counter=-243309312
w/call  3.13 clk/loop counter=-243309312
w/call  3.13 clk/loop counter=-243309312
cycles                                 9273429088    4.637 /loop
instructions                          10021217088    5.011 /loop
uops_executed.thread                  20025553553   10.013 /loop
uops_retired.retire_slots             12024036002    6.012 /loop
ld_blocks.store_forward                     14549    0.000 /loop
machine_clears.memory_ordering                104    0.000 /loop
uops_dispatched_port.port_2            2837364117    1.419 /loop
uops_dispatched_port.port_3            3132850688    1.566 /loop
uops_dispatched_port.port_4            6655729523    3.328 /loop
uops_dispatched_port.port_7            2038055794    1.019 /loop

=== nocall ===
wo/call 3.68 clk/loop counter=-243309312
wo/call 3.67 clk/loop counter=-243309312
wo/call 3.66 clk/loop counter=-243309312
wo/call 3.66 clk/loop counter=-243309312
cycles                                10837602138    5.419 /loop
instructions                           6022348710    3.011 /loop
uops_executed.thread                  10030569016    5.015 /loop
uops_retired.retire_slots              6029551051    3.015 /loop
ld_blocks.store_forward                     16851    0.000 /loop
machine_clears.memory_ordering                103    0.000 /loop
uops_dispatched_port.port_2            1455903313    0.728 /loop
uops_dispatched_port.port_3            1621561711    0.811 /loop
uops_dispatched_port.port_4            9817579933    4.909 /loop
uops_dispatched_port.port_7             932023637    0.466 /loop

// Xeon w9-3495
misc% sudo sh call-nocall-perf.sh 0
=== call ===
w/call  3.13 clk/loop counter=-243309312
w/call  3.21 clk/loop counter=-243309312
w/call  3.19 clk/loop counter=-243309312
w/call  3.21 clk/loop counter=-243309312
cycles                                14748038726    7.374 /loop
instructions                          10011129254    5.006 /loop
uops_executed.thread                  18012760992    9.006 /loop
uops_retired.slots                    12012546523    6.006 /loop
ld_blocks.store_forward                     11141    0.000 /loop
machine_clears.memory_ordering                348    0.000 /loop
uops_dispatched.port_2_3_10            4003460555    2.002 /loop
uops_dispatched.port_4_9               7683255440    3.842 /loop
uops_dispatched.port_7_8               4001892569    2.001 /loop

=== nocall ===
wo/call 3.18 clk/loop counter=-243309312
wo/call 3.16 clk/loop counter=-243309312
wo/call 3.17 clk/loop counter=-243309312
wo/call 3.16 clk/loop counter=-243309312
cycles                                14771366855    7.386 /loop
instructions                           6011265496    3.006 /loop
uops_executed.thread                  10012938599    5.006 /loop
uops_retired.slots                     6012704736    3.006 /loop
ld_blocks.store_forward                     11196    0.000 /loop
machine_clears.memory_ordering                372    0.000 /loop
uops_dispatched.port_2_3_10            2003443366    1.002 /loop
uops_dispatched.port_4_9              11109949422    5.555 /loop
uops_dispatched.port_7_8               2001881705    1.001 /loop

-DRMW=0
// Xeon 8280
split rmw
w/call  2.86 clk/loop counter=-243309312
wo/call 3.70 clk/loop counter=-243309312
w/call  2.78 clk/loop counter=-243309312
wo/call 3.69 clk/loop counter=-243309312

// Xeon w9-3495
split rmw
w/call  1.65 clk/loop counter=-243309312
wo/call 0.45 clk/loop counter=-243309312
w/call  1.88 clk/loop counter=-243309312
wo/call 0.44 clk/loop counter=-243309312
*/
#include <xbyak/xbyak_util.h>

const int  N = 100 * 1000 * 1000 * 10;
int counter = 0;

void (*loopCall)();
void (*loopNoCall)();

#ifndef RMW
#define RMW 1
#endif

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
#if RMW == 1
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
/*
	usage: call-nocall [call|nocall]
	no arg : run both (original behavior)
	call   : run loopCall only (for perf stat)
	nocall : run loopNoCall only (for perf stat)
*/
int main(int argc, char *argv[])
{
#if RMW == 1
	puts("rmw");
#else
	puts("split rmw");
#endif
	const char *mode = argc > 1 ? argv[1] : "";
	bool runCall = strcmp(mode, "nocall") != 0;
	bool runNoCall = strcmp(mode, "call") != 0;
	if (runCall) test("w/call ", loopCall);
	if (runNoCall) test("wo/call", loopNoCall);
	if (runCall) test("w/call ", loopCall);
	if (runNoCall) test("wo/call", loopNoCall);
}
