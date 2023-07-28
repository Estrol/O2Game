#include <windows.h>
#include <stdexcept>
#include <filesystem>
#include "Rendering/Renderer.h"
#include "Texture/NumericTexture.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

NumericTexture::NumericTexture(std::vector<ID3D11ShaderResourceView*>& numericsTexture) {
	if (numericsTexture.size() != 10) {
		throw std::runtime_error("NumericTexture::NumericTexture: numericsTexture.size() != 10");
	}

	Position2 = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };

	m_numericsTexture.resize(10);
	for (int i = 0; i < 10; i++) {
		auto tex = numericsTexture[i];
		//m_numericsTexture[i] = new Texture2D(tex);
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

	std::string numberString = std::to_string(number);
	if (MaxDigits != 0 && numberString.size() > MaxDigits) {
		numberString = numberString.substr(numberString.size() - MaxDigits, MaxDigits);
	}
	else {
		while (numberString.size() < MaxDigits && FillWithZeros) {
			numberString = "0" + numberString;
		}
	}

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
				tex->AlphaBlend = AlphaBlend;
				tex->AnchorPoint = AnchorPoint;
				tex->Draw();
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
				tex->AlphaBlend = AlphaBlend;
				tex->AnchorPoint = AnchorPoint;
				tex->Draw();
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
				tex->AlphaBlend = AlphaBlend;
				tex->Draw();
				tx += (int)m_numbericsWidth[digit].right + (int)(m_numbericsWidth[digit].right * offsetScl);
			}
			break;
		}

		default: {
			throw std::runtime_error("Invalid NumericPosition");
		}
	}
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
