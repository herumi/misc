#include <thread>
#include <stdio.h>
#include <fenv.h>
#include <math.h>

#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>

struct Code : Xbyak::CodeGenerator {
	Code(int mode)
	{
		roundsd(xm0, xm0, mode);
		ret();
	}
};

Code c0(0), c1(1), c2(2), c3(3);
double (*f0)(double) = c0.getCode<double (*)(double)>();
double (*f1)(double) = c1.getCode<double (*)(double)>();
double (*f2)(double) = c2.getCode<double (*)(double)>();
double (*f3)(double) = c3.getCode<double (*)(double)>();

void fff(int x, double d0, double d1)
{
	fesetround(x);
	printf("fff %d %f %f\n", x, nearbyint(d0), nearbyint(d1));

}

int main()
	try
{
	printf("         %f %f\n", 1.5, -1.5);
	printf("nearest  %f %f\n", f0(1.5), f0(-1.5));
	printf("to_minus %f %f\n", f1(1.5), f1(-1.5));
	printf("to_plus  %f %f\n", f2(1.5), f2(-1.5));
	printf("zero     %f %f\n", f3(1.5), f3(-1.5));

	fff(FE_TONEAREST, 1.5, -1.5);
	fff(FE_DOWNWARD, 1.5, -1.5);
	fff(FE_UPWARD, 1.5, -1.5);
	fff(FE_TOWARDZERO, 1.5, -1.5);
	std::thread t0(fff, FE_TONEAREST, 1.5, -1.5);
	std::thread t1(fff, FE_DOWNWARD, 1.5, -1.5);
	std::thread t2(fff, FE_UPWARD, 1.5, -1.5);
	std::thread t3(fff, FE_TOWARDZERO, 1.5, -1.5);
	t0.join();
	t1.join();
	t2.join();
	t3.join();
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
}
