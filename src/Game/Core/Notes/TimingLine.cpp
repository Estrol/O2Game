/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../Drawable/Sprite.h"
#include "../Resources/NoteImages.h"
#include "../RhythmEngine.h"
#include "TimingLine.h"
#include <UI/Image.h>

TimingLine::TimingLine()
{
    auto config = Resources::NoteImages::Get(NoteImageType::MEASURE_LINE);

    m_line = std::make_shared<Sprite>(config->Texture, config->TexCoords, config->FrameRate);
    m_line->Size = UDim2::fromOffset(0, config->ImagesRect.Height);
    m_line->Position = UDim2::fromOffset(0, 0);
    m_line->Color3 = config->Color;
}

TimingLine::~TimingLine()
{
}

void TimingLine::Load(TimingLineDesc timing)
{
    m_engine = timing.Engine;

    m_line->SpriteBatch = m_engine->GetMeasureSpriteBatch();

    m_offset = timing.Offset;
    m_startTime = timing.StartTime;
    m_currentTrackPosition = 0;
    m_imagePos = timing.ImagePos;
    m_imageSize = timing.ImageSize;
}

double TimingLine::GetOffset() const
{
    return m_offset;
}

double TimingLine::GetStartTime() const
{
    return m_startTime;
}

double TimingLine::GetTrackPosition() const
{
    return m_currentTrackPosition;
}

void TimingLine::Update(double delta)
{
    m_currentTrackPosition = m_engine->GetTrackPosition() - m_offset;
}

double CalculateLinePosition(double trackOffset, double offset, double noteSpeed, bool upscroll = false)
{
    return trackOffset + (offset * (upscroll ? -noteSpeed : noteSpeed) / 100.0);
}

void TimingLine::Draw(double delta)
{
    auto resolution = m_engine->GetResolution();
    auto hitPos = m_engine->GetHitPosition();
    auto playRect = m_engine->GetPlayRectangle();

    double alpha = CalculateLinePosition(1000.0, m_currentTrackPosition, m_engine->GetNotespeed()) / 1000.0;

    double min = 0, max = hitPos;
    double pos_y = min + (max - min) * alpha;

    m_line->Size = UDim2::fromOffset(playRect.Width - playRect.X, m_line->Size.Y.Offset);
    m_line->Position = UDim2::fromOffset(m_imagePos, pos_y); //+ start.Lerp(end, alpha);

    if (m_line->Position.Y.Offset >= 0 && m_line->Position.Y.Offset < hitPos + 10) {
        m_line->Draw(delta, playRect);
    }
}
