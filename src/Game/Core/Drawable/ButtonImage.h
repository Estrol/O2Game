/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once

#include "Sprite.h"
#include <functional>

#include <glm/glm.hpp>
#include <utility>
#include <vector>

class ButtonImage
{
public:
    ButtonImage();
    ButtonImage(Rect clickArena, std::shared_ptr<Sprite> image);

    void Draw();
    void Draw(Rect rect);

    bool UpdateInput();
    bool IsHovered();

    void OnClick(std::function<void()> callback);

private:
    Rect                  m_ClickArena;
    std::function<void()> m_Callback;

    std::shared_ptr<Sprite> m_Image;
};