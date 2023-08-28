#include "VelocityTiming.h"
#include "../../Data/Chart.hpp"
#include <cmath>

VelocityTiming::VelocityTiming(std::vector<TimingInfo>& _timings, std::vector<TimingInfo>& _velocities, double base) : TimingBase(_timings, _velocities, base) {
    if (velocities.size() > 0) {
        double pos = std::round(velocities[0].StartTime * base_multiplier * 100.0);
        offsets.push_back(pos);
		
        for (int i = 1; i < velocities.size(); i++) {
            pos += std::round((velocities[i].StartTime - velocities[i - 1].StartTime) * (velocities[i - 1].Value * 100));

			offsets.push_back(pos);
        }
    }
}

double VelocityTiming::GetOffsetAt(double offset) {
    int idx;

    for (idx = 0; idx < velocities.size(); idx++) {
        if (offset < velocities[idx].StartTime) {
            break;
        }
    }

    return GetOffsetAt(offset, idx);
}

double VelocityTiming::GetOffsetAt(double offset, int index) {
    if (index == 0) {
        return offset * base_multiplier * 100.0;
    }

    index -= 1;
    double pos = offsets[index];
    pos += (offset - velocities[index].StartTime) * (velocities[index].Value * 100.0);

    return pos;
}
