#pragma once
#include <vector>
#include "../../Data/Chart.hpp"

class TimingBase {
public:
	TimingBase(std::vector<TimingInfo>& _timings, std::vector<TimingInfo>& _velocities, double base);
	virtual ~TimingBase() = default;

	virtual double GetBeatAt(double offset);

	virtual double GetOffsetAt(double offset);
	virtual double GetOffsetAt(double offset, int index);
protected:
	std::vector<TimingInfo> timings;
	std::vector<TimingInfo> velocities;
	double base_multiplier = 1.0;
};