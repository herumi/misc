#pragma once

#include <memory.h>

union fi {
	uint32_t i;
	float f;
};

struct FloatFormat {
    uint32_t f:23;
    uint32_t e:8;
    uint32_t s:1;
	FloatFormat(uint32_t s = 0, uint32_t e = 0, uint32_t f = 0)
		: f(f)
		, e(e)
		, s(s)
	{
	}
	void set(uint32_t s_, uint32_t e_, uint32_t f_)
	{
		s = s_;
		e = e_;
		f = f_;
	}
	fi get() const
	{
		fi fi;
		memcpy(&fi, this, sizeof(*this));
		return fi;
	}
	void put() const
	{
		fi fi = get();
		printf("%08x %d %f %e\n", fi.i, fpclassify(fi.f), fi.f, fi.f);
	}
};
