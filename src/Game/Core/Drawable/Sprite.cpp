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
    Size = UDim2::fromScale(1, 1);
    Position = UDim2::fromOffset(0, 0);
    TintColor = Color3::fromRGB(255, 255, 255);
    AlphaBlend = false;
    Repeat = true;
    UpdateOnDraw = true;
    AnchorPoint = { 0, 0 };

    m_ShouldDispose = false;
    m_CurrentIndex = 0;
    m_RepeatIndex = 0;
    m_MaxIndex = 0;
    m_Delay = 0.0f;
    m_CurrentTime = 0.0f;
}

Sprite::Sprite(std::vector<std::filesystem::path> paths, float fps)
    : Sprite::Sprite(paths, 0, fps)
{
}

Sprite::Sprite(std::vector<Image *> paths, float fps)
    : Sprite::Sprite(paths, 0, fps)
{
}

Sprite::Sprite(std::vector<std::filesystem::path> paths, int repeatIndex, float fps)
    : Sprite::Sprite()
{
    for (auto &path : paths) {
        m_Images.push_back(new Image(path));
    }

    m_RepeatIndex = repeatIndex;
    m_MaxIndex = (int)m_Images.size() - 1;
    m_Delay = 1.0f / fps;

    m_ShouldDispose = true;
}

Sprite::Sprite(std::vector<Image *> paths, int repeatIndex, float fps)
    : Sprite::Sprite()
{
    m_Images = paths;

    m_RepeatIndex = repeatIndex;
    m_MaxIndex = (int)m_Images.size() - 1;
    m_Delay = 1.0f / fps;
}

Sprite::~Sprite()
{
    if (m_ShouldDispose) {
        for (auto &image : m_Images) {
            delete image;
        }
    }
}

void Sprite::Update(double fixedDelta)
{
    m_CurrentTime += static_cast<float>(fixedDelta);

    if (m_CurrentTime >= m_Delay) {
        m_CurrentTime = 0;

        m_CurrentIndex++;
    }

    if (Repeat && m_CurrentIndex >= m_Images.size()) {
        m_CurrentIndex = m_RepeatIndex;
    }
}

void Sprite::Draw(double delta)
{
    auto window = Graphics::NativeWindow::Get();
    auto windowSize = window->GetBufferSize();

    Draw(delta, { 0, 0, windowSize.Width, windowSize.Height });
}

void Sprite::Draw(double delta, Rect clip)
{
    if (UpdateOnDraw) {
        Update(delta);
    }

    if (m_CurrentIndex < m_Images.size()) {
        CalculateSize();

        auto &frame = m_Images[m_CurrentIndex];

        frame->AlphaBlend = AlphaBlend;
        frame->Color3 = TintColor;

        if (m_CurrentIndex != 0) {
            frame->Position = UDim2::fromOffset(AbsolutePosition.X, AbsolutePosition.Y);
            frame->Size = UDim2::fromOffset(AbsoluteSize.X, AbsoluteSize.Y);
            frame->AnchorPoint = { 0, 0 };
        }

        frame->Draw(clip);
    }
}

void Sprite::SetIndexAt(int index)
{
    m_CurrentIndex = index;
}

void Sprite::SetDelay(float fps)
{
    m_Delay = 1.0f / fps;
}

void Sprite::SetRepeatIndex(int index)
{
    m_RepeatIndex = index;
}

void Sprite::Reset()
{
    m_CurrentIndex = 0;
}

void Sprite::ResetLast()
{
    m_CurrentIndex = (int)m_Images.size() - 1;
}

Image **Sprite::GetImages()
{
    return &m_Images[0];
}

void Sprite::CalculateSize()
{
    m_Images[0]->AnchorPoint = AnchorPoint;
    m_Images[0]->Size = Size;
    m_Images[0]->Position = Position - Position2;
    m_Images[0]->CalculateSize();

    AbsoluteSize = m_Images[0]->AbsoluteSize;
    AbsolutePosition = m_Images[0]->AbsolutePosition;
}