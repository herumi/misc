#include <mcl/gmp_util.hpp>
#include "mcl.hpp"

#ifdef _MSC_VER
	#pragma warning(disable : 4127)
#endif

Code s_mcl;

int mcl_init(int n)
	try
{
	s_mcl.init(n);
	return 0;
} catch (std::exception& e) {
	fprintf(stderr, "err=%s\n", e.what());
	return 1;
}


