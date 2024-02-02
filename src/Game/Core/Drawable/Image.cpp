/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "Image.h"
#include <Graphics/NativeWindow.h>

#include "../../Env.h"

Image::Image() : UI::Image::Image()
{
    AlphaBlend = false;
}

Image::Image(std::filesystem::path path) : UI::Image::Image(path)
{
    AlphaBlend = false;
}

Image::Image(Graphics::Texture2D *texture) : UI::Image::Image(texture)
{
    AlphaBlend = false;
}

Image::Image(const char *buf, size_t size) : UI::Image::Image(buf, size)
{
    AlphaBlend = false;
}

Image::Image(const char *pixbuf, uint32_t width, uint32_t height) : UI::Image::Image(pixbuf, width, height)
{
    AlphaBlend = false;
}

void Image::Draw()
{
    if (AlphaBlend) {
        BlendState = Env::GetInt("BlendAlpha");
    } else {
        BlendState = Env::GetInt("BlendNonAlpha");
    }

    UI::Image::Draw();
}

void Image::Draw(Rect rect)
{
    if (AlphaBlend) {
        BlendState = Env::GetInt("BlendAlpha");
    } else {
        BlendState = Env::GetInt("BlendNonAlpha");
    }

    UI::Image::Draw(rect);
}

void Image::CalculateSize()
{
    Rect   imageRect = GetImageSize();
    Rect   bufferRect = Graphics::NativeWindow::Get()->GetBufferSize();
    double ImageWidth = imageRect.Width;
    double ImageHeight = imageRect.Height;
    double Width = bufferRect.Width;
    double Height = bufferRect.Height;

    double X = 0;
    double Y = 0;

    if (Parent != nullptr && Parent != this) {
        Parent->CalculateSize();

        Width = Parent->AbsoluteSize.X;
        Height = Parent->AbsoluteSize.Y;
        X = Parent->AbsolutePosition.X;
        Y = Parent->AbsolutePosition.Y;
    }

    double x0 = (Width * Position.X.Scale) + Position.X.Offset;
    double y0 = (Height * Position.Y.Scale) + Position.Y.Offset;

    double x1 = (ImageWidth * Size.X.Scale) + Size.X.Offset;
    double y1 = (ImageHeight * Size.Y.Scale) + Size.Y.Offset;

    double xAnchor = x1 * std::clamp(AnchorPoint.X, 0.0, 1.0);
    double yAnchor = y1 * std::clamp(AnchorPoint.Y, 0.0, 1.0);

    x0 -= xAnchor;
    y0 -= yAnchor;

    AbsolutePosition = { (X + x0), (Y + y0) };
    AbsoluteSize = { x1, y1 };

    roundedCornerPixels = glm::vec4();
}