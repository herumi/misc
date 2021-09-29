#include <mcl/gmp_util.hpp>
#include "code.hpp"

#ifdef _MSC_VER
	#pragma warning(disable : 4127)
#endif

int main()
{
	mpz_class p("12345");
	Code code;
	code.init(p);
}

