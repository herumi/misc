#include <stdio.h>

const int initValue = 100;
#if CPP17

template<class... Args>
auto sumR(Args...  args)
{
	return (args + ...);
}

template<class... Args>
auto sumL(Args...  args)
{
	return (... + args);
}
template<class... Args>
auto sumRI(Args...  args)
{
	return (args + ... + initValue);
}

template<class... Args>
auto sumLI(Args...  args)
{
	return (initValue + ... + args);
}

struct A {
	int a;
	A(int a) : a(a) {}
};

A operator+(const A& lhs, const A& rhs)
{
	printf("lhs=%d, rhs=%d\n", lhs.a, rhs.a);
	return A(lhs.a + rhs.a);
}

#else

auto sum()
{
  return initValue;
}

template<class Arg, class... Args>
auto sum(Arg head, Args... tails)
{
  return head + sum(tails...);
}

#endif

int main()
{
#ifdef CPP17
	printf("sumR =%d\n", sumR(A(1), A(2), A(3)).a);
	printf("sumL =%d\n", sumL(A(1), A(2), A(3)).a);
	printf("sumRI=%d\n", sumRI(A(1), A(2), A(3)).a);
	printf("sumLI=%d\n", sumLI(A(1), A(2), A(3)).a);
#else
	printf("sum=%d\n", sum(1, 2, 5, 4));
#endif
}
