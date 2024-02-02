/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "TimingBase.h"

class StaticTiming : public TimingBase
{
public:
    StaticTiming(std::vector<TimingInfo> &_timings, std::vector<TimingInfo> &_velocities, double base);
    ~StaticTiming() override = default;

    double GetOffsetAt(double offset) override;
    double GetOffsetAt(double offset, int index) override;
};