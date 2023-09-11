#include "FrameTimer.hpp"
#include "Texture/Texture2D.h"
#include "Rendering/Renderer.h"

FrameTimer::FrameTimer() {
	Repeat = false;
	m_currentFrame = 0;
	m_frameTime = 1.0f / 60;
	m_currentTime = 0;
	AlphaBlend = false;
	Size = UDim2::fromScale(1, 1);
	TintColor = { 1.0f, 1.0f, 1.0f };
}

FrameTimer::FrameTimer(std::vector<Texture2D*> frames) : FrameTimer::FrameTimer() {
	m_frames = frames;
}

FrameTimer::FrameTimer(std::vector<std::string> frames) : FrameTimer::FrameTimer() {
	m_frames = std::vector<Texture2D*>();
	for (auto frame : frames) {
		m_frames.push_back(new Texture2D(frame));
	}
}

FrameTimer::FrameTimer(std::vector<std::filesystem::path> frames) : FrameTimer::FrameTimer() {
	m_frames = std::vector<Texture2D*>();
	for (auto frame : frames) {
		m_frames.push_back(new Texture2D(frame));
	}

}

FrameTimer::FrameTimer(std::vector<SDL_Texture*> frames) : FrameTimer::FrameTimer() {
	m_frames = std::vector<Texture2D*>();
	for (auto frame : frames) {
		m_frames.push_back(new Texture2D(frame));
	}
}

FrameTimer::FrameTimer(std::vector<Texture2D_Vulkan*> frames) : FrameTimer::FrameTimer() {
	m_frames = std::vector<Texture2D*>();
	for (auto frame : frames) {
		m_frames.push_back(new Texture2D(frame));
	}
}

FrameTimer::~FrameTimer() {
	for (auto& f : m_frames) {
		delete f;
	}
}

void FrameTimer::Draw(double delta) {
	Draw(delta, nullptr);
}

void FrameTimer::Draw(double delta, Rect* clip) {
	m_currentTime += delta;

	if (m_currentTime >= m_frameTime) {
		m_currentTime -= m_frameTime;

		m_currentFrame++;
	}

	if (Repeat && m_currentFrame >= m_frames.size()) {
		m_currentFrame = 0;
	}

	if (m_currentFrame < m_frames.size()) {
		CalculateSize();
		
		m_frames[m_currentFrame]->AlphaBlend = AlphaBlend;
		m_frames[m_currentFrame]->TintColor = TintColor;
		if (m_currentFrame != 0) {
			m_frames[m_currentFrame]->Position = UDim2::fromOffset(AbsolutePosition.X, AbsolutePosition.Y);
			m_frames[m_currentFrame]->Size = UDim2::fromOffset(AbsoluteSize.X, AbsoluteSize.Y);
			m_frames[m_currentFrame]->AnchorPoint = { 0, 0 };
		}

		m_frames[m_currentFrame]->Draw(clip);
	}
}

void FrameTimer::SetFPS(float fps) {
	m_frameTime = 1.0f / fps;
}

void FrameTimer::ResetIndex() {
	m_currentFrame = 0;
}

void FrameTimer::LastIndex() {
	m_currentFrame = (int)m_frames.size() - 1;
}

void FrameTimer::SetIndexAt(int idx) {
	m_currentFrame = idx;
}

void FrameTimer::CalculateSize() {
	m_frames[0]->AnchorPoint = AnchorPoint;
	m_frames[0]->Size = Size;
	m_frames[0]->Position = Position;
	m_frames[0]->CalculateSize();

	AbsoluteSize = m_frames[0]->AbsoluteSize;
	AbsolutePosition = m_frames[0]->AbsolutePosition;
}
