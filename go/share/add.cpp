#if defined(_MSC_VER)
	#define DLL_EXPORT
	#ifdef DLL_EXPORT
		#define DLL_API __declspec(dllexport)
	#else
		#define DLL_API __declspec(dllimport)
	#endif
#else
	#define DLL_API
#endif

extern "C" DLL_API int add(int x, int y)
{
	return x + y;
}