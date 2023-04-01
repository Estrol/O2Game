#include "TimingLine.hpp"
#include "RhythmEngine.hpp"

namespace {
	float GetLinePositionAlpha(double trackOffset, double currentTrackPos, double noteSpeed, bool upscroll = false) {
		double pos = (trackOffset + (currentTrackPos * (upscroll ? -noteSpeed : noteSpeed) / 100.0)) / 1000.0;
		return -(1 - 0.8) + pos;
	}
}

TimingLine::TimingLine() {
	m_line = new DrawableLine();
	m_engine = nullptr;
}

TimingLine::~TimingLine() {
	Release();
}

void TimingLine::Load(TimingLineDesc* timing) {
	m_engine = timing->Engine;

	m_offset = timing->Offset;
	m_startTime = timing->StartTime;
	m_currentTrackPosition = 0;
	m_imagePos = timing->ImagePos;
	m_imageSize = timing->ImageSize;
}

void TimingLine::Update(double delta) {
	m_currentTrackPosition = m_engine->GetTrackPosition() - m_offset;
}

double TimingLine::GetOffset() const {
	return m_offset;
}

double TimingLine::GetStartTime() const {
	return m_startTime;
}

double TimingLine::GetTrackPosition() const {
	return m_currentTrackPosition;
}

void TimingLine::Render(double delta) {
	float alpha = GetLinePositionAlpha(1000, m_currentTrackPosition, m_engine->GetNotespeed());

	UDim2 start = UDim2::fromOffset(0, 0);
	UDim2 end = UDim2::fromOffset(0, 600);

	m_line->Size = UDim2::fromOffset(m_imageSize, 1);
	m_line->Position = UDim2::fromOffset(m_imagePos, 0) + start.Lerp(end, alpha);
	m_line->Draw(false);
}

void TimingLine::Release() {
	delete m_line;
}
