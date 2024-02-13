/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Graphics/Renderer.h>
#include <UI/Rectangle.h>

using namespace UI;
using namespace Graphics;

Rectangle::Rectangle() : Base()
{
    std::vector<unsigned char> pixels(200 * 200 * 4);
    for (int i = 0; i < 200 * 200 * 4; i++) {
        pixels[i] = 255;
    }

    m_texture = Renderer::Get()->LoadTexture(
        pixels.data(),
        200,
        200);
}

void Rectangle::OnDraw()
{
    using namespace Graphics::Backends;
    CalculateSize();

    double x1 = AbsolutePosition.X;
    double y1 = AbsolutePosition.Y;
    double x2 = x1 + AbsoluteSize.X;
    double y2 = y1 + AbsoluteSize.Y;

    glm::vec2 uv1(0.0f, 0.0f); // Top-left UV coordinate
    glm::vec2 uv2(1.0f, 0.0f); // Top-right UV coordinate
    glm::vec2 uv3(1.0f, 1.0f); // Bottom-right UV coordinate
    glm::vec2 uv4(0.0f, 1.0f); // Bottom-left UV coordinate
    glm::vec4 corRad(
        CornerRadius.X,
        CornerRadius.Y,
        CornerRadius.Z,
        CornerRadius.W);

    if (m_SubmitInfo.indices.size() != 6) {
        m_SubmitInfo.vertices.resize(6);
        m_SubmitInfo.indices = { 0, 1, 2, 3, 4, 5 };
    }

    glm::vec4 color = {
        Color3.R * 255,
        Color3.G * 255,
        Color3.B * 255,
        Transparency * 255
    };

    uint32_t col = ((uint32_t)(color.a) << 24) | ((uint32_t)(color.b) << 16) | ((uint32_t)(color.g) << 8) | ((uint32_t)(color.r) << 0);

    shaderFragmentType = ShaderFragmentType::Solid;

    m_SubmitInfo.vertices = {
        { { x1, y1 }, uv1, col },
        { { x1, y2 }, uv4, col },
        { { x2, y2 }, uv3, col },

        { { x1, y1 }, uv1, col },
        { { x2, y2 }, uv3, col },
        { { x2, y1 }, uv2, col },
    };
}