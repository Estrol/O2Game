/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "../../Data/chart.hpp"
#include <vector>

namespace Autoplay {
    enum class ReplayHitType {
        KEY_DOWN,
        KEY_UP
    };

    struct ReplayHitInfo
    {
        double                  Time;
        int                     Lane;
        Autoplay::ReplayHitType Type;
    };

    std::vector<Autoplay::ReplayHitInfo> CreateReplay(Chart *chart);
}; // namespace Autoplay