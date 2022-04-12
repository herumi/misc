//#include "fmath-sve.hpp"
//#include "log.hpp"
#include <algorithm>
#include <math.h>
#include <stdio.h>

void logsoftmax2(float x0, float x1)
{
	float maxv = std::max(x0, x1);
	float sum = 0;
	sum += std::exp(x0 - maxv) + std::exp(x1 - maxv);
	sum = std::log(sum);
	printf("0 : %.8f\n", x0 - maxv - sum);
	printf("1 : %.8f\n", x1 - maxv - sum);
}

int main()
{
	// 0 : -0.31326166
	// 1 : -1.31326163
	logsoftmax2(90, 89);
}
