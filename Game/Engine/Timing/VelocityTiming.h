#pragma once
#include "TimingBase.h"

class VelocityTiming : public TimingBase {
public:
	VelocityTiming(std::vector<TimingInfo>& _timings, double base);

	double GetBeatAt(double offset) override;
	double GetOffsetAt(double offset) override;
	double GetOffsetAt(double offset, int index) override;

private:
	std::vector<double> offsets;
};