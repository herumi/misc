#include <stdio.h>
#ifdef PRECOMPILE_HEADER
  #define INLINE_INST
#else
  #define INLINE_INST inline
#endif

INLINE_INST void X::put() const
{
	puts("AAA");
}
