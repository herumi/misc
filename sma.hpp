#pragma once
/*
	simple moving average
*/
#include <list>
#include <stdint.h>
#include <stdlib.h>
#include <cybozu/exception.hpp>

class SMAverage {
public:
	struct Val {
		uint64_t byteSize;
		double curTimeSec;
		Val(uint64_t byteSize = 0, double curTimeSec = 0)
			: byteSize(byteSize)
			, curTimeSec(curTimeSec)
		{
		}
	};
	typedef std::list<Val> ValVec;
private:
	ValVec vv_;
	double intevalSec_;
	uint64_t totalByte_;
	void removeOldElement(double curTimeSec)
	{
		for (;;) {
			ValVec::iterator begin = vv_.begin();
			if (begin == vv_.end()) break;
			if (begin->curTimeSec + intevalSec_ >= curTimeSec) break;
			totalByte_ -= begin->byteSize;
			vv_.pop_front();
		}
	}
public:
	explicit SMAverage(double intervalSec)
		: intevalSec_(intervalSec)
		, totalByte_(0)
	{
		if (intervalSec <= 0) throw cybozu::Exception("SMAverate:bad intervalSec") << intervalSec;
	}
	void append(uint64_t byteSize, double curTimeSec)
	{
		removeOldElement(curTimeSec);
		vv_.push_back(Val(byteSize, curTimeSec));
		totalByte_ += byteSize;
	}
	double getBps(double curSec)
	{
		removeOldElement(curSec);
		if (vv_.empty()) return 0;
		double delta = curSec - vv_.front().curTimeSec;
		if (delta == 0) delta = 1;
		return totalByte_ * 8 / delta;
	}
	const ValVec& getValVec() const { return vv_; }
	uint64_t getTotalByte() const { return totalByte_; }
};
