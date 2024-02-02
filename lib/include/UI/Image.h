/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __IMAGE_H_
#define __IMAGE_H_

#include "UIBase.h"
#include <filesystem>

namespace Graphics {
    class Texture2D;
} // namespace Graphics

namespace UI {
    enum class ImageScaleMode {
        Stretch,
        Fit,
        FitWindow
    };

    class Image : public Base
    {
    public:
        Image();
        Image(std::filesystem::path path);
        Image(Graphics::Texture2D *texture);
        Image(const char *buf, size_t size);
        Image(const char *pixbuf, uint32_t width, uint32_t height);

        Rect           GetImageSize() const;
        ImageScaleMode ScaleMode = ImageScaleMode::Stretch;

    protected:
        void OnDraw() override;

        void CalculateSize() override;
    };
} // namespace UI

#endif