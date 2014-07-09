#pragma once
/*
	simple moving average
*/
#include <list>
#include <stdint.h>
#include <time.h>

class SMAverage {
public:
	struct Val {
		uint64_t v;
		uint64_t t;
		Val(uint64_t v = 0, uint64_t t = 0)
			: v(v)
			, t(t)
		{
		}
	};
	typedef std::list<Val> ValVec;
private:
	ValVec vv_;
	uint64_t interval_;
	uint64_t totalVal_;
	size_t num_;
	void removeOldElement(uint64_t t)
	{
		for (;;) {
			typename ValVec::iterator begin = vv_.begin();
			if (begin == vv_.end()) break;
			if (begin->t + interval_ >= t) break;
			totalVal_ -= begin->v;
			vv_.pop_front();
			num_--;
		}
	}
public:
	explicit SMAverage(uint64_t interval)
		: interval_(interval)
		, totalVal_(0)
		, num_(0)
	{
	}
	void append(uint64_t v, uint64_t t)
	{
		removeOldElement(t);
		vv_.push_back(Val(v, t));
		totalVal_ += v;
		num_++;
	}
	double get() const
	{
		return totalVal_ / double(num_);
	}
	const ValVec& getValVec() const { return vv_; }
	uint64_t getTotalVal() const { return totalVal_; }
};
