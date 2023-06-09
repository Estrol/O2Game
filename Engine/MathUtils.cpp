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
