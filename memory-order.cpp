#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <thread>

const int N = 100000;
struct X {
	int a;
	int b;
	int r1;
	int r2;
	X() : a(0), b(0), r1(0), r2(0) {}
};

X x[N];

void (*p0)();
void (*p1)();

struct Code : Xbyak::CodeGenerator {
	void proc0()
	{
		mov(rax, (size_t)x);
		for (int i = 0; i < N; i++) {
			int adj = i * (int)sizeof(X);
			mov(dword [rax + adj + 0], 1); // mov [X], 1
			mov(rcx, ptr [rax + adj + 4]); // mov r1, [B]
			mov(dword [rax + adj + 8], rcx); // r1
		}
		ret();
	}
	void proc1()
	{
		mov(rax, (size_t)x);
		for (int i = 0; i < N; i++) {
			int adj = i * (int)sizeof(X);
			mov(dword [rax + adj + 4], 1); // mov [B], 1
			mov(rcx, ptr [rax + adj + 0]); // mov r1, [X]
			mov(dword [rax + adj + 12], rcx); // r2
		}
		ret();
	}
	Code()
		: Xbyak::CodeGenerator(4096 * 1280)
	{
		p0 = getCurr<void (*)()>();
		proc0();
		align(16);
		p1 = getCurr<void (*)()>();
		proc1();
	}
};

void verify()
{
	int count[2][2] = {};
	for (int i = 0; i < N; i++) {
		count[x[i].r1][x[i].r2]++;
	}
	printf("r 0 0 : %d\n", count[0][0]);
	printf("r 0 1 : %d\n", count[0][1]);
	printf("r 1 0 : %d\n", count[1][0]);
	printf("r 1 1 : %d\n", count[1][1]);
}

int main()
	try
{
	Code code;
//	verify();
	std::thread t0(p1);
	std::thread t1(p0);
	puts("run");
	t0.join();
	t1.join();
	puts("verify");
	verify();
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}

