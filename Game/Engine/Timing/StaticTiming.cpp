#include "StaticTiming.h"

double StaticTiming::GetBeatAt(double offset) {
    return 0.0;
}

double StaticTiming::GetOffsetAt(double offset) {
    return GetOffsetAt(offset, 0);
}

double StaticTiming::GetOffsetAt(double offset, int index) {
    return offset * 100.0;
}
