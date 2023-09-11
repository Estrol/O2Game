#pragma once
#include "UDim2.h"
#include "../Imgui/imgui.h"

namespace MathUtil {
	ImVec2 ScaleVec2(ImVec2 input);
	ImVec2 ScaleVec2(double x, double y);

	double Lerp(double min, double max, double alpha);
}