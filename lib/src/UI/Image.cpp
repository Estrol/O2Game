/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Graphics/NativeWindow.h>
#include <Graphics/Renderer.h>
#include <UI/Image.h>
using namespace UI;
using namespace Graphics;

Image::Image()
{
}

Image::Image(std::filesystem::path path)
{
    auto renderer = Renderer::Get();
    auto image = renderer->LoadTexture(path);

    m_texture = std::unique_ptr<Texture2D>(image);
}

Image::Image(Graphics::Texture2D *texture)
{
    m_texture = std::unique_ptr<Texture2D>(texture);
}

Image::Image(const char *buf, size_t size)
{
    auto renderer = Renderer::Get();
    auto image = renderer->LoadTexture(buf, size);

    m_texture = std::unique_ptr<Texture2D>(image);
}

Image::Image(const char *pixbuf, uint32_t width, uint32_t height)
{
    auto renderer = Renderer::Get();
    auto image = renderer->LoadTexture(pixbuf, width, height);

    m_texture = std::unique_ptr<Texture2D>(image);
}

Rect Image::GetImageSize() const
{
    if (m_texturePtr) {
        return m_texturePtr->GetSize();
    } else {
        return m_texture->GetSize();
    }
}

void Image::OnDraw()
{
    using namespace Backends;
    CalculateSize();

    double x1 = AbsolutePosition.X;
    double y1 = AbsolutePosition.Y;
    double x2 = x1 + AbsoluteSize.X;
    double y2 = y1 + AbsoluteSize.Y;

    glm::vec2 uv1(0.0f, 0.0f); // Top-left UV coordinate
    glm::vec2 uv2(1.0f, 0.0f); // Top-right UV coordinate
    glm::vec2 uv3(1.0f, 1.0f); // Bottom-right UV coordinate
    glm::vec2 uv4(0.0f, 1.0f); // Bottom-left UV coordinate

    glm::vec4 color = {
        Color3.R * 255,
        Color3.G * 255,
        Color3.B * 255,
        Transparency * 255
    };

    uint32_t col = ((uint32_t)(color.a) << 24) | ((uint32_t)(color.b) << 16) | ((uint32_t)(color.g) << 8) | ((uint32_t)(color.r) << 0);

    shaderFragmentType = ShaderFragmentType::Image;

    m_SubmitInfo.indices = { 0, 1, 2, 3, 4, 5 };
    m_SubmitInfo.vertices = {
        { { x1, y1 }, uv1, col },
        { { x1, y2 }, uv4, col },
        { { x2, y2 }, uv3, col },

        { { x1, y1 }, uv1, col },
        { { x2, y2 }, uv3, col },
        { { x2, y1 }, uv2, col },
    };
}

void Image::CalculateSize()
{
    UI::Base::CalculateSize();

    if (ScaleMode == ImageScaleMode::Fit || ScaleMode == ImageScaleMode::FitWindow) {
        auto window = Graphics::NativeWindow::Get();

        Rect windowSize;
        if (ScaleMode == ImageScaleMode::FitWindow) {
            windowSize = window->GetWindowSize();
        } else {
            windowSize = window->GetBufferSize();
        }

        float aspectRatio = (float)AbsoluteSize.X / (float)AbsoluteSize.Y;
        float newWidth = (float)windowSize.Width;
        float newHeight = newWidth / aspectRatio;

        if (newHeight > (float)windowSize.Height) {
            newHeight = (float)windowSize.Height;
            newWidth = newHeight * aspectRatio;
        }

        Vector2 newAbsolutePosition = {
            ((windowSize.Width - newWidth) / 2.0f),
            ((windowSize.Height - newHeight) / 2.0f)
        };

        Vector2 newAbsoluteSize = {
            newWidth,
            newHeight
        };

        if (ScaleMode == ImageScaleMode::FitWindow) {
            Vector2 windowScale = window->GetWindowScale();

            newAbsoluteSize.X /= windowScale.X;
            newAbsoluteSize.Y /= windowScale.Y;

            newAbsolutePosition.X /= windowScale.X;
            newAbsolutePosition.Y /= windowScale.Y;
        }

        AbsoluteSize = newAbsoluteSize;
        AbsolutePosition = newAbsolutePosition;
    }
}