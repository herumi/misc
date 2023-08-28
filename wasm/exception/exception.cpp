#include <stdexcept>

#if defined(__EMSCRIPTEN__)
	#define API __attribute__((used))
#elif defined(__wasm__)
	#define API __attribute__((visibility("default")))
#endif

extern "C" {
API int func_nocatch(int x);
API int func_catch(int x);
}

int func_nocatch(int x)
{
	if (x < 0) throw std::runtime_error("bad x");
	return x * 2;
}

int func_catch(int x)
{
	try {
		return func_nocatch(x);
	} catch (...) {
		return -1;
	}
}
