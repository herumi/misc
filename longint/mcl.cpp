#include <mcl/gmp_util.hpp>
#include "mcl.hpp"

#ifdef _MSC_VER
	#pragma warning(disable : 4127)
#endif

Code s_mcl;

int mcl_init(const char *pStr)
	try
{
	s_mcl.init(pStr);
	return 0;
} catch (std::exception& e) {
	fprintf(stderr, "err=%s\n", e.what());
	return 1;
}


