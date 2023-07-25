#include "TimingBase.h"
#include "../../Data/Chart.hpp"

TimingBase::TimingBase(std::vector<TimingInfo>& _timings, double base) {
    timings = _timings;
    base_multiplier = base;
}

double TimingBase::GetBeatAt(double offset) {
    return 0.0;
}

double TimingBase::GetOffsetAt(double offset) {
    return 0.0;
}

double TimingBase::GetOffsetAt(double offset, int index) {
    return 0.0;
}
