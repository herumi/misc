#include <iostream>
#include <stdio.h>
#include <type_traits>
#include <typeinfo>

template<class T>
void put(const T& x)
{
	printf("type=%s\n", typeid(T).name());
	if constexpr (std::is_integral_v<T>) {
		printf("int %d\n", x);
	} else if constexpr (std::is_convertible_v<T, const char*>) {
		printf("str %s\n", x);
	} else {
		std::cout << "other " << x << std::endl;
	}
}

int main()
{
	put(123);
	put("abc");
	put(1.2);
}
