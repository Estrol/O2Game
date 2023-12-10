#pragma once
#include "TimingBase.h"

class VelocityTiming : public TimingBase
{
public:
    VelocityTiming(std::vector<TimingInfo> &_timings, std::vector<TimingInfo> &_velocities, double base);
    ~VelocityTiming() override = default;

    double GetOffsetAt(double offset) override;
    double GetOffsetAt(double offset, int index) override;

private:
    std::vector<double> offsets;
};