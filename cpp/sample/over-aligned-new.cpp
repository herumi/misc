#include <new>
#include <memory>
#include <stdint.h>

struct alignas(64) X {
	uint32_t a[16];
};

int main()
{
	X *p = new X();
	printf("p=%p\n", p);
}
