/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once

#include "Image.h"
#include <functional>

#include <utility>

class ButtonImage
{
public:
    ButtonImage();
    ButtonImage(Rect clickArena, std::pair<std::shared_ptr<Image>, std::shared_ptr<Image>> images);

    void Draw();
    void Draw(Rect rect);

    bool UpdateInput();
    bool IsHovered();

    void OnClick(std::function<void()> callback);

private:
    Rect                                                      m_ClickArena;
    std::function<void()>                                     m_Callback;
    std::pair<std::shared_ptr<Image>, std::shared_ptr<Image>> m_Images;
};