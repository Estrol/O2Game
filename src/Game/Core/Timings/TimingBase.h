/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "../../Data/Chart.hpp"
#include <vector>

class TimingBase
{
public:
    TimingBase(std::vector<TimingInfo> &_timings, std::vector<TimingInfo> &_velocities, double base);
    virtual ~TimingBase() = default;

    virtual double GetBeatAt(double offset);
    virtual double GetBPMAt(double offset);

    virtual double GetOffsetAt(double offset);
    virtual double GetOffsetAt(double offset, int index);

protected:
    std::vector<TimingInfo> timings;
    std::vector<TimingInfo> velocities;
    double                  base_multiplier = 1.0;
};