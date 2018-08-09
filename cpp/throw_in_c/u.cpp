#include <stdio.h>
#include <stdexcept>

extern "C" void (*get())(void f());
void (*getCpp())();

int main()
	try
{
	void (*g)(void f()) = get();
	void (*f)() = getCpp();
	puts("callback");
	g(f);
} catch (std::exception& e) {
	printf("e=%s\n", e.what());
}
