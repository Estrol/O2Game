#include "TimingBase.h"

TimingBase::TimingBase(std::vector<TimingInfo>& _timings, std::vector<TimingInfo>& _velocities, double base) {
    timings = _timings;
    velocities = _velocities;
    base_multiplier = base;
}

double TimingBase::GetBeatAt(double offset) {
    int min = 0, max = timings.size() - 1;
    int left = min, right = max;

    while (left <= right) {
        int mid = (left + right) / 2;

        bool afterMid = mid < 0 || timings[mid].StartTime < offset;
        bool beforeMid = mid + 1 >= timings.size() || timings[mid + 1].StartTime > offset;

        if (afterMid && beforeMid) {
            return timings[mid].CalculateBeat(offset);
        } else if (afterMid) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    if (timings.size() == 0) return 0;

    return timings[0].CalculateBeat(offset);
}


double TimingBase::GetOffsetAt(double offset) {
    return GetOffsetAt(offset, 0);
}

double TimingBase::GetOffsetAt(double offset, int index) {
    return 0;
}
