#include "VelocityTiming.h"
#include "../../Data/Chart.hpp"

VelocityTiming::VelocityTiming(std::vector<TimingInfo>& _timings, double base) : TimingBase(_timings, base) {
    if (timings.size() > 0) {
        double pos = std::round(timings[0].StartTime * base_multiplier * 100.0);
        offsets.push_back(pos);
		
        for (int i = 1; i < timings.size(); i++) {
            pos += std::round((timings[i].StartTime - timings[i - 1].StartTime) * (timings[i - 1].Value * 100));

			offsets.push_back(pos);
        }
    }
}

double VelocityTiming::GetBeatAt(double offset) {
    return 0.0;
}

double VelocityTiming::GetOffsetAt(double offset) {
    int idx;

    for (int i = 0; i < timings.size(); i++) {
        if (offsets[i] > offset) {
            idx = i;
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
    pos += (offset - timings[index].StartTime) * (timings[index].Value * 100.0);

    return pos;
}
