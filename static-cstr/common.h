#pragma once

#ifndef PRIORITY
	#define PRIORITY 65535
#endif

#ifdef __GNUC__
	#define CSTR __attribute__((constructor(PRIORITY)))
	#define INIT_PRIORITY(x) __attribute__((init_priority(x)))
#else
	#define CSTR
	#define INIT_PRIORITY(x)
#endif
