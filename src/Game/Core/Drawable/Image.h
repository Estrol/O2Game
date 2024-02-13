/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once

#include <UI/Image.h>

class Image : public UI::Image
{
public:
    Image();
    Image(std::filesystem::path path);
    Image(std::shared_ptr<Graphics::Texture2D> texture);
    Image(const char *buf, size_t size);
    Image(const char *pixbuf, uint32_t width, uint32_t height);

    bool AlphaBlend;

    void Draw() override;
    void Draw(Rect rect) override;

    void CalculateSize() override;
};