#pragma once
#include <string>
#include <functional>
#include "../../Engine/Data/WindowsTypes.hpp"

class Button {
public:
	Button() = default;
	Button(
		int x, 
		int y, 
		int width, 
		int height, 
		std::function<void(int)> mouse_hover, 
		std::function<void()> mouse_click
	);
	~Button();

	void Render(double delta);
	void Input(double delta);
	bool OnKeyDown();

private:
	int m_x;
	int m_y;
	int m_width;
	int m_height;
	std::function<void()> m_mouseClick;
	std::function<void(int)> m_mouseHover;

	bool IsOutside = false;
	Rect m_lastState;
};