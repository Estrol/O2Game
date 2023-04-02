#pragma once
#include <string>
#include <vector>

namespace BMS {
	struct BMSEvent {
		int Measure;
		int Channel;

		std::vector<std::string> Params;
	};
}