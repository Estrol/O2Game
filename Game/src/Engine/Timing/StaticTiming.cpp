#include "StaticTiming.h"

StaticTiming::StaticTiming(std::vector<TimingInfo>& _timings, std::vector<TimingInfo>& _velocities, double base) : TimingBase(_timings, _velocities, base) {

}

double StaticTiming::GetOffsetAt(double offset) {
    return GetOffsetAt(offset, 0);
}

double StaticTiming::GetOffsetAt(double offset, int index) {
    return offset * 100.0;
}
