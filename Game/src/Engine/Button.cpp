#include "Button.hpp"
#include "Imgui/ImguiUtil.h"
#include "Imgui/imgui.h"
#include "Inputs/InputManager.h"
#include "Texture/Bitmap.h"
#include <Texture/MathUtils.h>

#if __LINUX__
#define TRUE 1
#define FALSE 0
#endif

struct Vector2
{
    LONG X, Y;
};

bool IsRectInsideRect(const Vector2 &innerRect, const Rect &outerRect)
{
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

Button::Button(int x, int y, int width, int height)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
}

Button::Button(int x, int y, int width, int height, std::function<void(int)> mouse_hover, std::function<void()> mouse_click)
    : Button(x, y, width, height)
{

    OnMouseClick = mouse_click;
    OnMouseHover = mouse_hover;

    m_lastState = {};
}

Button::~Button()
{
    OnMouseClick = nullptr;
    OnMouseHover = nullptr;
}

bool Button::IsHovered()
{
    return m_isHovered;
}

void Button::Render(double delta)
{
    Rect    targetRect = { m_x, m_y, m_x + m_width, m_y + m_height };
    Vector2 pos = { m_lastState.left, m_lastState.top };

    if (IsRectInsideRect(pos, targetRect)) {
        ImguiUtil::BeginText(ImVec2((float)m_x, (float)m_y), ImVec2((float)m_width, (float)m_height));

        auto draw_list = ImGui::GetWindowDrawList();

        ImColor col = { 255, 255, 255, 255 };
        if (!m_isHovered) {
            col = { 255, 0, 0, 255 };
        }

        draw_list->AddRectFilled(ImVec2((float)m_x, (float)m_y), ImVec2((float)m_width, (float)m_height), ImColor(255, 255, 255, 255));

        ImguiUtil::EndText();
    }
}

void Button::Input(double delta)
{
    InputManager *inputs = InputManager::GetInstance();

    m_lastState = inputs->GetMousePosition();

    Rect    targetRect = { m_x, m_y, m_x + m_width, m_y + m_height };
    Vector2 pos = { m_lastState.left, m_lastState.top };

    if (IsRectInsideRect(pos, targetRect)) {
        bool previousClick = m_isClicked;
        m_isClicked = inputs->IsMouseButton(MouseButton::LEFT);

        if (!previousClick && m_isClicked) {
            if (OnMouseClick) {
                OnMouseClick();
            }
        } else if (previousClick && !m_isClicked) {
            m_isClicked = false;
        }

        if (!m_isHovered) {
            m_isHovered = true;

            if (OnMouseHover) {
                OnMouseHover(TRUE);
            }
        }
    } else {
        if (m_isHovered) {
            m_isHovered = false;

            if (OnMouseHover) {
                OnMouseHover(FALSE);
            }
        }
    }
}
