#include "MathUtils.hpp"
#include "Window.hpp"

ImVec2 MathUtil::ScaleVec2(ImVec2 input) {
	Window* wnd = Window::GetInstance();
	if (wnd->IsScaleOutput()) {
		input.x *= wnd->GetWidthScale();
		input.y *= wnd->GetHeightScale();
	}

	return input;
}

ImVec2 MathUtil::ScaleVec2(double x, double y) {
	ImVec2 input(x, y);

	Window* wnd = Window::GetInstance();
	if (wnd->IsScaleOutput()) {
		input.x *= wnd->GetWidthScale();
		input.y *= wnd->GetHeightScale();
	}

	return input;
}

double MathUtil::Lerp(double min, double max, double alpha) {
	return min * (1.0 - alpha) + (max * alpha);
}
