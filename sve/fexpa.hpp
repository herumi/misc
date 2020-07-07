#pragma once

#include <math.h>
#include <stdint.h>

struct FexpaTbl {
	static const size_t N = 64;
	uint32_t tbl[N];
	union fi {
		float f;
		uint32_t i;
	};
	
	uint32_t mask(int n) const
	{
		if (n == 32) return uint32_t(-1);
		return (1u << n) - 1;
	}
	FexpaTbl()
	{
		for (size_t i = 0; i < N; i++) {
			fi fi;
			fi.f = (float)pow(2.0, i / float(N));
			tbl[i] = fi.i & mask(23);
		}
	}
	float calc(float f) const
	{
		fi fi;
		fi.f = f;
		uint32_t v = fi.i;
		fi.i = ((v >> 6) & mask(8)) << 23;
		fi.i |= tbl[v & 63];
		return fi.f;
	}
};

float fexpaEmu(float x)
{
	static FexpaTbl tbl;
	return tbl.calc(x);
}
