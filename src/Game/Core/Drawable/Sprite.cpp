/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../../Env.h"
#include "Sprite.h"
#include <Graphics/NativeWindow.h>
#include <Graphics/Renderer.h>

Sprite::Sprite()
{
    AlphaBlend = false;
    UpdateOnDraw = true;
    Repeat = true;

    m_frameTime = 1.0f;
    m_elapsedFrameTime = 0.0;
    m_spriteIndex = 0;
    m_RepeatIndex = 0;
}

Sprite::Sprite(
    std::filesystem::path               path,
    std::vector<std::vector<glm::vec2>> texCoords,
    float                               fps)
    : UI::Sprite(path, texCoords, fps)
{
    AlphaBlend = false;
    UpdateOnDraw = true;
    Repeat = true;

    m_frameTime = 1.0 / fps;
    m_elapsedFrameTime = 0.0;
    m_spriteIndex = 0;
    m_RepeatIndex = 0;
}

Sprite::Sprite(
    std::shared_ptr<Graphics::Texture2D> texture,
    std::vector<std::vector<glm::vec2>>  texCoords,
    float                                fps)
    : UI::Sprite(texture, texCoords, fps)
{
    AlphaBlend = false;
    UpdateOnDraw = true;
    Repeat = true;

    m_frameTime = 1.0 / fps;
    m_elapsedFrameTime = 0.0;
    m_spriteIndex = 0;
    m_RepeatIndex = 0;
}

void Sprite::Update(double fixedDelta)
{
    m_elapsedFrameTime += fixedDelta;

    if (m_elapsedFrameTime >= m_frameTime) {
        m_elapsedFrameTime = 0.0;
        m_spriteIndex++;

        if (m_spriteIndex >= m_texCoords.size()) {
            if (Repeat) {
                m_spriteIndex = m_RepeatIndex;
            } else {
                m_spriteIndex = (int)(m_texCoords.size());
            }
        }
    }
}

void Sprite::Draw()
{
    auto window = Graphics::NativeWindow::Get();
    auto windowSize = window->GetBufferSize();

    Draw(0, windowSize);
}

void Sprite::Draw(Rect rc)
{
    Draw(0, rc);
}

void Sprite::Draw(double delta)
{
    auto window = Graphics::NativeWindow::Get();
    auto windowSize = window->GetBufferSize();

    Draw(delta, windowSize);
}

void Sprite::Draw(double delta, Rect clip)
{
    if (UpdateOnDraw) {
        Update(delta);
    }

    if (m_spriteIndex < m_texCoords.size()) {
        if (AlphaBlend) {
            BlendState = Env::GetInt("BlendAlpha");
        } else {
            BlendState = Env::GetInt("BlendNonAlpha");
        }

        Image::Draw(clip);
    }
}

void Sprite::CalculateSize()
{
    UDim2 PositionToCalculate = Position - Position2;

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

    double x0 = (Width * PositionToCalculate.X.Scale) + PositionToCalculate.X.Offset;
    double y0 = (Height * PositionToCalculate.Y.Scale) + PositionToCalculate.Y.Offset;

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

void Sprite::SetIndexAt(int index)
{
    m_spriteIndex = index;
}

void Sprite::SetDelay(float fps)
{
    m_frameTime = 1.0 / fps;
}

void Sprite::SetRepeatIndex(int index)
{
    m_RepeatIndex = index;
}

void Sprite::Reset()
{
    m_spriteIndex = 0;
}

void Sprite::ResetLast()
{
    m_spriteIndex = (int)(m_texCoords.size() - 1);
}