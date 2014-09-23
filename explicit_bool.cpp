#include <stdio.h>
#if (__cplusplus >= 201103L) || (_MSC_VER >= 1800)
#define USE_EXPLICT_BOOL
#endif

class A {
	const char *p_;
public:
	A(const char *p) : p_(p) {}
	operator bool() const
	{
		return p_ != 0;
	}
};

class X {
	const char *p_;
	class BoolType {
		void operator delete(void*);
	};
public:
	X(const char *p) : p_(p) {}
	operator const BoolType *() const
	{
		static BoolType b;
		return p_ ? &b : 0;
	}

};

class Y {
	const char *p_;
	typedef void (Y::*BoolType)() const;
	void dummy() const {}
public:
	Y(const char *p) : p_(p) {}
	operator BoolType() const
	{
		return p_ ? &Y::dummy : 0;
	}
};

#ifdef USE_EXPLICT_BOOL
class Z {
	const char *p_;
public:
	Z(const char *p) : p_(p) {}
	explicit operator bool() const
	{
		return p_ != 0;
	}
	bool operator==(int) const { return p_ != 0; }
	bool operator!=(int) const { return p_ == 0; }
};
#endif

extern "C" bool aaa(const char *p)
{
	A a(p);
	return a;
}
extern "C" bool xxx(const char *p)
{
	X a(p);
	return a;
}
extern "C" bool yyy(const char *p)
{
	Y a(p);
	return a;
}
#ifdef USE_EXPLICT_BOOL
extern "C" bool zzz(const char *p)
{
	Z a(p);
	return (bool)a;
}
#endif
int main(int, char *argv[])
{
	{
		A a(argv[0]);
		if (a) {
			puts("A0");
		}
		if (a != 0) {
			puts("A1");
		}
		if (a != NULL) {
			puts("A2");
		}
	}
	{
		X x(argv[0]);
		if (x) {
			puts("X0");
		}
		if (x != 0) {
			puts("X1");
		}
		if (x != NULL) {
			puts("X2");
		}
	}
	{
		Y y(argv[0]);
		if (y) {
			puts("Y0");
		}
		if (y != 0) {
			puts("Y1");
		}
		if (y != NULL) {
			puts("Y2");
		}
	}
#ifdef USE_EXPLICT_BOOL
	{
		Z z(argv[0]);
		if (z) {
			puts("Z0");
		}
		if (z != 0) {
			puts("Z1");
		}
		if (z != NULL) {
			puts("Z2");
		}
	}
#endif
}
