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
		m_frames.push_back(new Texture2D(frame));
	}
	
	m_currentFrame = 0;
	m_frameTime = 1.0f / 60.0;
	m_currentTime = 0;
	Repeat = false;
}

FrameTimer::FrameTimer(std::vector<std::filesystem::path> frames) {
	m_frames = std::vector<Texture2D*>();
	for (auto frame : frames) {
		m_frames.push_back(new Texture2D(frame));
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
		auto spriteBatch = Renderer::GetInstance()->GetSpriteBatch();
		auto device = Renderer::GetInstance()->GetDevice();
		auto context = Renderer::GetInstance()->GetImmediateContext();
		auto states = Renderer::GetInstance()->GetStates();

		spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, states->AlphaBlend(), states->PointWrap(), nullptr, nullptr, [&] {
			D3D11_BLEND_DESC blendDesc = {};
			blendDesc.AlphaToCoverageEnable = false;
			blendDesc.IndependentBlendEnable = false;
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			ID3D11BlendState* blendState;
			HRESULT hr = device->CreateBlendState(&blendDesc, &blendState);
			if (SUCCEEDED(hr)) {
				context->OMSetBlendState(blendState, nullptr, 0xffffffff);
				blendState->Release();
			}
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
