#include <stdint.h>
#ifdef __EMSCRIPTEN__
	#define API __attribute__((used))
#else
	#define API __attribute__((visibility("default")))
#endif
extern "C" API uint32_t add(uint32_t x, uint32_t y)
{
	return x + y;
}
