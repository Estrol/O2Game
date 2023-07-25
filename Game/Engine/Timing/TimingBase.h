#pragma once
#include <vector>
struct TimingInfo;

class TimingBase {
public:
	TimingBase(std::vector<TimingInfo>& _timings, double base);
	virtual ~TimingBase() = default;

	virtual double GetBeatAt(double offset);

	virtual double GetOffsetAt(double offset);
	virtual double GetOffsetAt(double offset, int index);
protected:
	std::vector<TimingInfo> timings;
	double base_multiplier = 1.0;
};