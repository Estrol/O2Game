/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Exceptions/EstException.h>
#include <Graphics/GraphicsTexture2D.h>
#include <Graphics/NativeWindow.h>
#include <Graphics/Renderer.h>
#include <UI/UIBase.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

using namespace UI;

Base::Base()
{
    Position = UDim2::fromScale(0.0, 0.0);
    Size = UDim2::fromScale(1.0, 1.0);
    AnchorPoint = { 0.0f, 0.0f };
    CornerRadius = { 0.0f, 0.0f, 0.0f, 0.0f };
    Color3 = Color3::fromRGB(255, 255, 255);
    Transparency = 1;
    Rotation = 0;
    Parent = nullptr;
    ClampToParent = false;
    BlendState = Graphics::Backends::DefaultBlend::BLEND;

    m_ScaleSize = true;
    m_renderMode = RenderMode::Normal;
    m_texturePtr = nullptr;
}

Base::~Base()
{
    // Automated garbage collector!
    if (m_texture) {
        m_texture.reset();
    }
}

void Base::Draw()
{
    clipRect = Graphics::NativeWindow::Get()->GetBufferSize();

    OnDraw();

    DrawVertices();
}

void Base::Draw(Rect _clipRect)
{
    clipRect = _clipRect;

    OnDraw();

    DrawVertices();
}

float CalculatePixelSize(float alpha, const glm::vec2 &rectSize)
{
    if (alpha <= 0)
        return 0;

    float diagonalSize = glm::length(rectSize);
    float pixelSize = alpha * diagonalSize;
    return pixelSize;
}

void Base::CalculateSize()
{
    if (Static && !NeedsUpdate()) {
        return;
    }

    // auto   windowRect = Graphics::NativeWindow::Get()->GetWindowSize();
    Rect   bufferRect = Graphics::NativeWindow::Get()->GetBufferSize();
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

    double x1 = (Width * Size.X.Scale) + Size.X.Offset;
    double y1 = (Height * Size.Y.Scale) + Size.Y.Offset;

    double xAnchor = x1 * std::clamp(AnchorPoint.X, 0.0, 1.0);
    double yAnchor = y1 * std::clamp(AnchorPoint.Y, 0.0, 1.0);

    x0 -= xAnchor;
    y0 -= yAnchor;

    AbsolutePosition = { (X + x0), (Y + y0) };
    AbsoluteSize = { x1, y1 };

    roundedCornerPixels = glm::vec4(
        CalculatePixelSize(static_cast<float>(CornerRadius.X), glm::vec2(x1, y1)),
        CalculatePixelSize(static_cast<float>(CornerRadius.Y), glm::vec2(x1, y1)),
        CalculatePixelSize(static_cast<float>(CornerRadius.Z), glm::vec2(x1, y1)),
        CalculatePixelSize(static_cast<float>(CornerRadius.W), glm::vec2(x1, y1)));
}

void Base::DrawVertices()
{
    using namespace Graphics::Backends;
    auto renderer = Graphics::Renderer::Get();

    if (m_renderMode == RenderMode::Normal) {
        auto windowRect = Graphics::NativeWindow::Get()->GetWindowSize();
        auto bufferRect = Graphics::NativeWindow::Get()->GetBufferSize();

        float widthRatio = static_cast<float>(windowRect.Width) / bufferRect.Width;
        float heightRatio = static_cast<float>(windowRect.Height) / bufferRect.Height;

        glm::vec2 absoluteSize = glm::vec2(AbsoluteSize.X * widthRatio, AbsoluteSize.Y * heightRatio);

        m_SubmitInfo.clipRect = clipRect;
        if (m_ScaleSize) {
            for (auto &vertex : m_SubmitInfo.vertices) {
                vertex.pos.x *= widthRatio;
                vertex.pos.y *= heightRatio;
            }

            m_SubmitInfo.clipRect = {
                static_cast<int>(m_SubmitInfo.clipRect.X * widthRatio),
                static_cast<int>(m_SubmitInfo.clipRect.Y * heightRatio),
                static_cast<int>(m_SubmitInfo.clipRect.Width * widthRatio),
                static_cast<int>(m_SubmitInfo.clipRect.Height * heightRatio)
            };
        }

        m_SubmitInfo.fragmentType = shaderFragmentType;
        m_SubmitInfo.alphablend = BlendState;
        m_SubmitInfo.uiSize = absoluteSize;
        m_SubmitInfo.uiRadius = roundedCornerPixels;

        if (m_texturePtr != nullptr) {
            m_SubmitInfo.image = m_texturePtr->GetId();
        } else if (m_texture) {
            m_SubmitInfo.image = m_texture->GetId();
        }

        // RotateVertex();

        if (SpriteBatch) {
            SpriteBatch->Draw(m_SubmitInfo);
        } else {
            renderer->Push(m_SubmitInfo);
        }
    } else {
        if (!m_batches.size()) {
            InsertToBatch();
        }

        // RotateVertex();

        if (SpriteBatch) {
            SpriteBatch->Draw(m_batches);
        } else {
            renderer->Push(m_batches);
        }
    }
}

void Base::InsertToBatch()
{
    using namespace Graphics::Backends;
    auto renderer = Graphics::Renderer::Get();

    if (m_renderMode == RenderMode::Normal) {
        return;
    }

    auto windowRect = Graphics::NativeWindow::Get()->GetWindowSize();
    auto bufferRect = Graphics::NativeWindow::Get()->GetBufferSize();

    float widthRatio = static_cast<float>(windowRect.Width) / bufferRect.Width;
    float heightRatio = static_cast<float>(windowRect.Height) / bufferRect.Height;

    glm::vec2 absoluteSize = glm::vec2(AbsoluteSize.X * widthRatio, AbsoluteSize.Y * heightRatio);

    m_SubmitInfo.clipRect = clipRect;
    if (m_ScaleSize) {
        for (auto &vertex : m_SubmitInfo.vertices) {
            vertex.pos.x *= widthRatio;
            vertex.pos.y *= heightRatio;
        }

        m_SubmitInfo.clipRect = {
            static_cast<int>(m_SubmitInfo.clipRect.X * widthRatio),
            static_cast<int>(m_SubmitInfo.clipRect.Y * heightRatio),
            static_cast<int>(m_SubmitInfo.clipRect.Width * widthRatio),
            static_cast<int>(m_SubmitInfo.clipRect.Height * heightRatio)
        };
    }

    m_SubmitInfo.fragmentType = shaderFragmentType;
    m_SubmitInfo.alphablend = BlendState;
    m_SubmitInfo.uiSize = absoluteSize;
    m_SubmitInfo.uiRadius = roundedCornerPixels;

    if (m_texturePtr != nullptr) {
        m_SubmitInfo.image = m_texturePtr->GetId();
    } else if (m_texture) {
        m_SubmitInfo.image = m_texture->GetId();
    }

    m_batches.push_back(m_SubmitInfo);
}

inline glm::vec2 computeCenter(std::vector<Graphics::Backends::Vertex> &vertices)
{
    glm::vec2 sum(0.0f, 0.0f);
    int       count = 0;

    for (const auto &vertex : vertices) {
        sum += vertex.pos;
        count++;
    }

    return sum / static_cast<float>(count);
}

inline glm::vec2 computeCenter(const std::vector<Graphics::Backends::SubmitInfo> &batches)
{
    glm::vec2 sum(0.0f, 0.0f);
    int       count = 0;

    for (const auto &batch : batches) {
        for (const auto &vertex : batch.vertices) {
            sum += vertex.pos;
            count++;
        }
    }

    return sum / static_cast<float>(count);
}

inline glm::vec2 rotate(glm::vec2 &vec, float cos, float sin)
{
    return { vec.x * cos - vec.y * sin, vec.x * sin + vec.y * cos };
}

void Base::RotateVertex()
{
    if (!m_vertices.size()) {
        throw Exceptions::EstException("Vertex array is empty");
    }

    if (Rotation == 0) {
        return;
    }

    float radians = glm::radians(Rotation);
    float cosAngle = glm::cos(radians);
    float sinAngle = glm::sin(radians);

    if (m_renderMode == RenderMode::Normal) {
        auto center = computeCenter(m_vertices);
        center = rotate(center, cosAngle, sinAngle) - center;

        for (auto &vertex : m_vertices) {
            vertex.pos = rotate(vertex.pos, cosAngle, sinAngle) - center;
        }
    } else {
        auto center = computeCenter(m_batches);
        center = rotate(center, cosAngle, sinAngle) - center;

        for (auto &info : m_batches) {
            for (auto &vertices : info.vertices) {
                vertices.pos = rotate(vertices.pos, cosAngle, sinAngle) - center;
            }
        }
    }
}

bool Base::NeedsUpdate()
{
    if (m_cache.Position != Position) {
        return true;
    }

    if (m_cache.Size != Size) {
        return true;
    }

    if (m_cache.Parent != Parent) {
        return true;
    }

    if (m_cache.ClampToParent != ClampToParent) {
        return true;
    }

    if (m_cache.Color3 != Color3) {
        return true;
    }

    if (m_cache.AnchorPoint != AnchorPoint) {
        return true;
    }

    if (m_cache.CornerRadius != CornerRadius) {
        return true;
    }

    if (m_cache.Transparency != Transparency) {
        return true;
    }

    if (m_cache.Rotation != Rotation) {
        return true;
    }

    return false;
}

void Base::UpdateCache()
{
    m_cache.AbsolutePosition = AbsolutePosition;
    m_cache.AbsoluteSize = AbsoluteSize;
    m_cache.Position = Position;
    m_cache.Size = Size;
    m_cache.Parent = Parent;
    m_cache.ClampToParent = ClampToParent;
    m_cache.Color3 = Color3;
    m_cache.AnchorPoint = AnchorPoint;
    m_cache.CornerRadius = CornerRadius;
    m_cache.Transparency = Transparency;
    m_cache.Rotation = Rotation;
}

void Base::OnDraw()
{
}