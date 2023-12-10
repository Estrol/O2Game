#include "TimingLine.hpp"
#include "RhythmEngine.hpp"
#include "Texture/ResizableImage.h"

namespace {
    double CalculateLinePosition(double trackOffset, double offset, double noteSpeed, bool upscroll = false)
    {
        return trackOffset + (offset * (upscroll ? -noteSpeed : noteSpeed) / 100.0);
    }
} // namespace

TimingLine::TimingLine()
{
    m_line = new ResizableImage(198, 1, (char)0xFF);
    m_engine = nullptr;
}

TimingLine::~TimingLine()
{
    Release();
}

void TimingLine::Load(TimingLineDesc *timing)
{
    m_engine = timing->Engine;

    m_offset = timing->Offset;
    m_startTime = timing->StartTime;
    m_currentTrackPosition = 0;
    m_imagePos = timing->ImagePos;
    m_imageSize = timing->ImageSize;
}

void TimingLine::Update(double delta)
{
    m_currentTrackPosition = m_engine->GetTrackPosition() - m_offset;
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

void TimingLine::Render(double delta)
{
    auto resolution = m_engine->GetResolution();
    auto hitPos = m_engine->GetHitPosition();
    auto playRect = m_engine->GetPlayRectangle();

    double alpha = CalculateLinePosition(1000.0, m_currentTrackPosition, m_engine->GetNotespeed()) / 1000.0;

    double min = 0, max = hitPos;
    double pos_y = min + (max - min) * alpha;

    m_line->Size = UDim2::fromOffset(m_imageSize, 1);
    m_line->Position = UDim2::fromOffset(m_imagePos, pos_y); //+ start.Lerp(end, alpha);

    if (m_line->Position.Y.Offset >= 0 && m_line->Position.Y.Offset < hitPos + 10) {
        m_line->Draw(&playRect);
    }
}

void TimingLine::Release()
{
    delete m_line;
}
