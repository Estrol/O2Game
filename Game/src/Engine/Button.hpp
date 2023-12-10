#pragma once
#include "Rendering/WindowsTypes.h"
#include <functional>
#include <string>

class Button
{
public:
    Button() = default;
    Button(
        int x,
        int y,
        int width,
        int height);
    Button(
        int                      x,
        int                      y,
        int                      width,
        int                      height,
        std::function<void(int)> mouse_hover,
        std::function<void()>    mouse_click);
    ~Button();

    std::function<void()>    OnMouseClick;
    std::function<void(int)> OnMouseHover;

    bool IsHovered();

    void Render(double delta);
    void Input(double delta);

private:
    int m_x;
    int m_y;
    int m_width;
    int m_height;

    bool m_isHovered = false;
    bool m_isClicked = false;

    Rect m_lastState;
};