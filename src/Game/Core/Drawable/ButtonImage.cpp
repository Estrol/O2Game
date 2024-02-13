/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "ButtonImage.h"

#include <Graphics/NativeWindow.h>
#include <Inputs/InputManager.h>

ButtonImage::ButtonImage()
{
    m_Callback = {};
}

ButtonImage::ButtonImage(Rect clickArena, std::shared_ptr<Sprite> image)
    : m_ClickArena(clickArena), m_Image(image)
{
    m_Callback = {};
}

void ButtonImage::Draw()
{
    Rect rc = Graphics::NativeWindow::Get()->GetBufferSize();

    Draw(rc);
}

void ButtonImage::Draw(Rect rect)
{
    if (IsHovered()) {
        m_Image->SetIndexAt(0);
    } else {
        m_Image->SetIndexAt(1);
    }

    m_Image->Draw(rect);
}

// clang-format off
bool IsVectorInsideRect(Vector2 vec, Rect rect)
{
    return vec.X >= rect.X 
        && vec.X <= rect.X + rect.Width 
        && vec.Y >= rect.Y 
        && vec.Y <= rect.Y + rect.Height;
}
// clang-format on

bool ButtonImage::IsHovered()
{
    auto mousePos = Inputs::Manager::Get()->GetMousePosition();
    auto window = Graphics::NativeWindow::Get();
    auto windowScale = window->GetWindowScale();

    Rect scaledRect = m_ClickArena;
    scaledRect.X = (int)(scaledRect.X * windowScale.X);
    scaledRect.Y = (int)(scaledRect.Y * windowScale.Y);
    scaledRect.Width = (int)(scaledRect.Width * windowScale.X);
    scaledRect.Height = (int)(scaledRect.Height * windowScale.Y);

    return IsVectorInsideRect(mousePos, scaledRect);
}

bool ButtonImage::UpdateInput()
{
    if (m_Callback) {
        if (IsHovered() && Inputs::Manager::Get()->IsMouseDown(Inputs::Mouse::Left)) {
            m_Callback();
            return true;
        }
    }

    return false;
}

void ButtonImage::OnClick(std::function<void()> callback)
{
    m_Callback = callback;
}