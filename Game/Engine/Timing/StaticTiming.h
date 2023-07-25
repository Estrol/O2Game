#pragma once
#include "TimingBase.h"

class StaticTiming : public TimingBase {
public:
	double GetBeatAt(double offset) override;
	double GetOffsetAt(double offset) override;
	double GetOffsetAt(double offset, int index) override;
};