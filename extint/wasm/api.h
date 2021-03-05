#pragma once

#ifdef __wasm__
	#define API __attribute__((visibility("default")))
#else
	#define API
#endif

