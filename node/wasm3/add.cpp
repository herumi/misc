#if defined(__wasm__)
	#define DLL_API __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
extern "C" {
#endif

DLL_API int add(int x, int y)
{
	return x + y;
}

#ifdef __cplusplus
}
#endif
