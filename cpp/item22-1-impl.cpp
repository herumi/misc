#include <string>
#include "item22-1.hpp"

struct Impl {
	std::string a;
	Impl()
		: a("abc")
	{
		puts("cstr");
	}
	~Impl()
	{
		puts("dstr");
	}
	Impl(const Impl& rhs)
		: a(rhs.a)
	{
		puts("cp cstr");
	}
	Impl(Impl&& rhs)
		: a(std::move(rhs.a))
	{
		puts("mv cstr");
	}
	Impl& operator=(const Impl& rhs)
	{
		a = rhs.a;
		puts("cp assign");
		return *this;
	}
	Impl& operator=(Impl&& rhs)
	{
		a = std::move(rhs.a);
		puts("mv assign");
		return *this;
	}
	void put() const
	{
		printf("a=%s\n", a.c_str());
	}
};

void A::put() const
{
	if (pImpl_ == nullptr) {
		puts("pImpl is nullptr");
		return;
	}
	pImpl_->put();
}

A::A() : pImpl_(std::unique_ptr<Impl>(new Impl()))
{
}

A::~A() = default;
A::A(A&&) = default;
A& A::operator=(A&&) = default;

A::A(const A& rhs)
	: pImpl_(std::unique_ptr<Impl>(rhs.pImpl_ ? new Impl(*rhs.pImpl_) : new Impl()))
{
}

A& A::operator=(const A& rhs)
{
	if (rhs.pImpl_) {
		if (pImpl_) {
			*pImpl_ = *rhs.pImpl_;
		} else {
			pImpl_.reset(new Impl(*rhs.pImpl_));
		}
	} else {
		pImpl_.reset();
	}
	return *this;
}

B::B() : pImpl_(std::make_shared<Impl>())
{
}
void B::put() const
{
	if (pImpl_ == nullptr) {
		puts("pImpl is nullptr");
		return;
	}
	pImpl_->put();
}
