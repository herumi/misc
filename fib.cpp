#include <stdio.h>
#include <stack>

std::stack<int> g_stack;
void push(int x)
{
	g_stack.push(x);
}
int pop()
{
	int v = g_stack.top();
	g_stack.pop();
	return v;
}

int I, N, R, R1, R2;

// input : N
// output : R
void fib()
{
	if (N <= 1) {
		R = 1;
		return;
	}
	push(N);
	N = N - 1;
	fib();
	R1 = R;
	N = pop();
	N = N - 2;
	push(R1);
	fib();
	R2 = R;
	R1 = pop();
	R = R1 + R2;
}

int main()
{
	for (I = 0; I <= 10; I++) {
		printf("fib(%d)=", I);
		N = I;
		fib();
		printf("%d\n", R);
	}
}
