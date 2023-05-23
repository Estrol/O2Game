#include <windows.h>
#include <stdexcept>
#include <filesystem>
#include "Renderer.hpp"
#include "NumericTexture.hpp"
#include <directxtk/WICTextureLoader.h>

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

using namespace DirectX;

NumericTexture::NumericTexture(std::vector<ID3D11ShaderResourceView*>& numericsTexture) {
	if (numericsTexture.size() != 10) {
		throw std::runtime_error("NumericTexture::NumericTexture: numericsTexture.size() != 10");
	}

	Position2 = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };

	m_numericsTexture.resize(10);
	for (int i = 0; i < 10; i++) {
		auto tex = numericsTexture[i];
		m_numericsTexture[i] = new Texture2D(tex);
		m_numbericsWidth[i] = m_numericsTexture[i]->GetOriginalRECT();
	}
}

NumericTexture::NumericTexture(std::vector<std::string> numericsFiles) {
	if (numericsFiles.size() != 10) {
		throw std::runtime_error("NumericTexture::NumericTexture: numericsFiles.size() != 10");
	}

	Position2 = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };

	m_numericsTexture.resize(10);
	for (int i = 0; i < 10; i++) {
		auto path = numericsFiles[i];
		m_numericsTexture[i] = new Texture2D(path);
		m_numbericsWidth[i] = m_numericsTexture[i]->GetOriginalRECT();
	}
}

NumericTexture::NumericTexture(std::vector<std::filesystem::path> numericsPath) {
	if (numericsPath.size() != 10) {
		throw std::runtime_error("NumericTexture::NumericTexture: numericsFiles.size() != 10");
	}

	Position2 = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };

	m_numericsTexture.resize(10);
	for (int i = 0; i < 10; i++) {
		auto path = numericsPath[i];
		m_numericsTexture[i] = new Texture2D(path);
		m_numbericsWidth[i] = m_numericsTexture[i]->GetOriginalRECT();
	}
}

NumericTexture::~NumericTexture() {
	for (auto& tex : m_numericsTexture) {
		delete tex;
	}
}

void NumericTexture::DrawNumber(int number) {
	Window* window = Window::GetInstance();
	Renderer* renderer = Renderer::GetInstance();
	auto batch = renderer->GetSpriteBatch();
	auto states = renderer->GetStates();
	auto context = renderer->GetImmediateContext();
	auto rasterizerState = renderer->GetRasterizerState();

	std::string numberString = std::to_string(number);
	if (MaxDigits != 0 && numberString.size() > MaxDigits) {
		numberString = numberString.substr(numberString.size() - MaxDigits, MaxDigits);
	}
	else {
		while (numberString.size() < MaxDigits && FillWithZeros) {
			numberString = "0" + numberString;
		}
	}

	batch->Begin(SpriteSortMode_Deferred, states->NonPremultiplied(), states->PointWrap(), nullptr, nullptr, [&] {
		if (AlphaBlend) {
			context->OMSetBlendState(renderer->GetBlendState(), nullptr, 0xffffffff);
		}
	});

	LONG xPos = static_cast<LONG>(window->GetBufferWidth() * Position.X.Scale) + static_cast<LONG>(Position.X.Offset);
	LONG yPos = static_cast<LONG>(window->GetBufferHeight() * Position.Y.Scale) + static_cast<LONG>(Position.Y.Offset);

	LONG xMPos = static_cast<LONG>(window->GetBufferWidth() * Position2.X.Scale) + static_cast<LONG>(Position2.X.Offset);
	LONG yMPos = static_cast<LONG>(window->GetBufferHeight() * Position2.Y.Scale) + static_cast<LONG>(Position2.Y.Offset);

	xPos += xMPos;
	yPos += yMPos;

	float offsetScl = (float)Offset / 100.0f;

	switch (NumberPosition) {
		case NumericPosition::LEFT: {
			int tx = xPos;
			for (int i = (int)numberString.length() - 1; i >= 0; i--) {
				int digit = numberString[i] - '0';

				tx -= (int)m_numbericsWidth[digit].right + (int)(m_numbericsWidth[digit].right * offsetScl);
				auto tex = m_numericsTexture[digit];
				tex->Position = UDim2({ 0, (float)tx }, { 0, (float)yPos });
				tex->AnchorPoint = AnchorPoint;
				tex->Draw(false);
			}
			break;
		}

		case NumericPosition::MID: {
			int totalWidth = 0;
			for (int i = 0; i < numberString.length(); i++) {
				int digit = numberString[i] - '0';
				totalWidth += (int)m_numbericsWidth[digit].right + (int)(m_numbericsWidth[digit].right * offsetScl);
			}
			
			int tx = xPos - totalWidth / 2 + (Offset * totalWidth) / 200;
			//int tx = xPos - (totalWidth / 2) * (totalWidth / 100);
			for (int i = 0; i < numberString.length(); i++) {
				int digit = numberString[i] - '0';
				auto tex = m_numericsTexture[digit];
				tex->Position = UDim2({ 0, (float)tx }, { 0, (float)yPos });
				tex->AnchorPoint = AnchorPoint;
				tex->Draw(false);
				tx += (int)m_numbericsWidth[digit].right + (int)(m_numbericsWidth[digit].right * offsetScl);
			}
			break;
		}

		case NumericPosition::RIGHT: {
			int tx = xPos;
			for (int i = 0; i < numberString.length(); i++) {
				int digit = numberString[i] - '0';
				auto tex = m_numericsTexture[digit];
				tex->Position = UDim2({ 0, (float)tx }, { 0, (float)yPos });
				tex->AnchorPoint = AnchorPoint;
				tex->Draw(false);
				tx += (int)m_numbericsWidth[digit].right + (int)(m_numbericsWidth[digit].right * offsetScl);
			}
			break;
		}

		default: {
			throw std::runtime_error("Invalid NumericPosition");
		}
	}

	batch->End();
}

void NumericTexture::SetValue(int value) { // Add SetValueinto DrawNumber for NumericTexture so it can update
	DrawNumber(value);
}

NumericPosition IntToPos(int i) {
	switch (i) {
		case -1: return NumericPosition::LEFT;
		case 0: return NumericPosition::MID;
		case 1: return NumericPosition::RIGHT;

		default: throw std::runtime_error("IntToPos: i is not a valid NumericPosition");
	}
}
