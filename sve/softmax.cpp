#define FMATH_USE_LOG_TBL
#include "fmath-sve.hpp"
#include "log.hpp"
#include <algorithm>
#include <math.h>
#include <stdio.h>

// 0 : -0.31326166
// 1 : -1.31326163
float logsoftmax1(float x0, float x1)
{
	float maxv = std::max(x0, x1);
	float sum = 0;
	sum += std::exp(x0 - maxv) + std::exp(x1 - maxv);
	sum = std::log(sum);
	float ret = x0 - maxv - sum;
	printf("0 : %.8f\n", ret);
	printf("1 : %.8f\n", x1 - maxv - sum);
	return ret;
}

// 0 : -0.31326175
// 1 : -1.31326175
float logsoftmax2(float x0, float x1)
{
	float maxv = std::max(x0, x1);
	float sum = 0;
	sum += fmath::expf(x0 - maxv) + fmath::expf(x1 - maxv);
	sum = fmath::logf(sum);
	float ret = x0 - maxv - sum;
	printf("0 : %.8f\n", ret);
	printf("1 : %.8f\n", x1 - maxv - sum);
	return ret;
}


int main()
{
	float ok = logsoftmax1(90, 89);
	float my = logsoftmax2(90, 89);
	printf("ok - my=%e\n", ok - my);
}
