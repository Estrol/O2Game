#include "Button.hpp"
#include "Inputs/InputManager.h"
#include "Imgui/ImguiUtil.h"
#include "Imgui/imgui.h"
#include "Texture/Bitmap.h"

#if __LINUX__
#define TRUE 1
#define FALSE 0
#endif

struct Vector2 {
	LONG X, Y;
};

bool IsRectInsideRect(const Vector2& innerRect, const Rect& outerRect) {
	if (innerRect.X < outerRect.left) {
		return false;
	}

	if (innerRect.X > outerRect.right) {
		return false;
	}

	if (innerRect.Y < outerRect.top) {
		return false;
	}

	if (innerRect.Y > outerRect.bottom) {
		return false;
	}

	return true;
}

Button::Button(int x, int y, int width, int height, std::function<void(int)> mouse_hover, std::function<void()> mouse_click) {
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;

	m_mouseClick = mouse_click;
	m_mouseHover = mouse_hover;

	m_lastState = {};
}

Button::~Button() {
	m_mouseClick = nullptr;
	m_mouseHover = nullptr;
}

void Button::Render(double delta) {
	Rect targetRect = { m_x, m_y, m_x + m_width, m_y + m_height };
	Vector2 pos = { m_lastState.left, m_lastState.top };

	if (IsRectInsideRect(pos, targetRect)) {
		// ImguiUtil::BeginText();

		// auto draw_list = ImGui::GetWindowDrawList();

		// draw_list->AddRect(ImVec2((float)targetRect.left, (float)targetRect.top), ImVec2((float)targetRect.right, (float)targetRect.bottom), ImColor(255, 255, 255, 255));

		// ImguiUtil::EndText();
	}
}

void Button::Input(double delta) {
	InputManager* inputs = InputManager::GetInstance();

	m_lastState = inputs->GetMousePosition();
	
	Rect targetRect = { m_x, m_y, m_x + m_width, m_y + m_height };
	Vector2 pos = { m_lastState.left, m_lastState.top };

	if (IsRectInsideRect(pos, targetRect) && !IsOutside) {
		IsOutside = true;
		m_mouseHover(TRUE);
	}
	else {
		if (IsOutside && !IsRectInsideRect(pos, targetRect)) {
			IsOutside = false;

			m_mouseHover(FALSE);
		}
	}
}

bool Button::OnKeyDown() {
	InputManager* inputs = InputManager::GetInstance();

	Rect targetRect = { m_x, m_y, m_x + m_width, m_y + m_height };
	Vector2 pos = { m_lastState.left, m_lastState.top };

	if (IsRectInsideRect(pos, targetRect)) {
		if (inputs->IsMouseButton(MouseButton::LEFT)) {
			m_mouseClick();
			return true;
		}
	}

	return false;
}
