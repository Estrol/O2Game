#include "FrameTimer.hpp"
#include <directxtk/SpriteBatch.h>
#include "../../Engine/Texture2D.hpp"
#include "../../Engine/Renderer.hpp"

FrameTimer::FrameTimer(std::vector<Texture2D*> frames) {
	m_frames = frames;
	Repeat = false;
	m_currentFrame = 0;
	m_frameTime = 1.0f / 60;
	m_currentTime = 0;
}

FrameTimer::FrameTimer(std::vector<std::string> frames) {
	m_frames = std::vector<Texture2D*>();
	for (auto frame : frames) {
		m_frames.emplace_back(new Texture2D(frame));
	}
	
	m_currentFrame = 0;
	m_frameTime = 1.0f / 60.0;
	m_currentTime = 0;
	Repeat = false;
}

FrameTimer::FrameTimer(std::vector<std::filesystem::path> frames) {
	m_frames = std::vector<Texture2D*>();
	for (auto frame : frames) {
		m_frames.emplace_back(new Texture2D(frame));
	}

	m_currentFrame = 0;
	m_frameTime = 1.0f / 60.0;
	m_currentTime = 0;
	Repeat = false;
}

FrameTimer::~FrameTimer() {
	for (auto& f : m_frames) {
		delete f;
	}
}

void FrameTimer::Draw(double delta) {
	m_currentTime += delta;

	if (m_currentTime >= m_frameTime) {
		m_currentTime -= m_frameTime;

		m_currentFrame++;
	}

	if (Repeat && m_currentFrame >= m_frames.size()) {
		m_currentFrame = 0;
	}

	if (m_currentFrame < m_frames.size()) {
		auto renderer = Renderer::GetInstance();
		auto spriteBatch = renderer->GetSpriteBatch();
		auto device = renderer->GetDevice();
		auto context = renderer->GetImmediateContext();
		auto states = renderer->GetStates();

		spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, states->NonPremultiplied(), states->PointWrap(), nullptr, nullptr, [&] {
			context->OMSetBlendState(renderer->GetBlendState(), nullptr, 0xffffffff);
		});

		m_frames[m_currentFrame]->AnchorPoint = AnchorPoint;
		m_frames[m_currentFrame]->Position = Position;
		m_frames[m_currentFrame]->Draw(false);

		spriteBatch->End();
	}
}

void FrameTimer::SetFPS(float fps) {
	m_frameTime = 1.0f / fps;
}

void FrameTimer::ResetIndex() {
	m_currentFrame = 0;
}

void FrameTimer::LastIndex() {
	m_currentFrame = m_frames.size() - 1;
}

void FrameTimer::SetIndexAt(int idx) {
	m_currentFrame = idx;
}
