#pragma once
#include "../Data/Chart.hpp"
#include <vector>

namespace Autoplay {
    enum class ReplayHitType {
        KEY_DOWN,
        KEY_UP
    };

    struct ReplayHitInfo {
        double Time;
        int Lane;
        Autoplay::ReplayHitType Type;
    };

    std::vector<Autoplay::ReplayHitInfo> CreateReplay(Chart* chart);
};