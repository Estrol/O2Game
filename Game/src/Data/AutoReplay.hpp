#pragma once
#include <vector>
#include "Chart.hpp"

enum class ReplayHitType {
	KEY_DOWN,
	KEY_UP
};

struct ReplayHitInfo {
	double Time;
	int Lane;
	ReplayHitType Type;
};

namespace AutoReplay {
	std::vector<ReplayHitInfo> CreateReplay(Chart* chart);
}