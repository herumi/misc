#include <mcl/gmp_util.hpp>
#include "code.hpp"
#include "code.h"

#ifdef _MSC_VER
	#pragma warning(disable : 4127)
#endif

Code s_code;

int code_init()
	try
{
	s_code.init();
	return 0;
} catch (std::exception& e) {
	fprintf(stderr, "err=%s\n", e.what());
	return 1;
}


