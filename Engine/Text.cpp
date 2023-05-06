#include "Text.hpp"
#include "Renderer.hpp"

Text::Text(std::filesystem::path textPath) {
	if (!std::filesystem::exists(textPath)) {
		throw std::runtime_error(("SpriteFont file: " + textPath.filename().string() + " is not found!").c_str());
	}

	auto device = Renderer::GetInstance()->GetDevice();
	m_font = new DirectX::SpriteFont(device, textPath.wstring().c_str());

	Size = UDim2::fromScale(1, 1);
	TintColor = { 1, 1, 1 };
}

void Text::Draw(std::wstring text) {
	Draw(text, true);
}

void Text::Draw(std::wstring text, bool manualDraw) {
	Draw(text, nullptr, manualDraw);
}

void Text::Draw(std::wstring text, RECT* clipRect) {
	Draw(text, clipRect, true);
}

void Text::Draw(std::wstring text, RECT* clipRect, bool manualDraw) {
	using namespace DirectX;

	RECT textSize = m_font->MeasureDrawBounds(text.c_str(), DirectX::XMFLOAT2(0, 0));
	
	auto renderer = Renderer::GetInstance();
	auto batch = renderer->GetSpriteBatch();
	auto states = renderer->GetStates();
	auto context = renderer->GetImmediateContext();
	auto rasterizerState = renderer->GetRasterizerState();
	

	Window* window = Window::GetInstance();
	int wWidth = window->GetWidth();
	int wHeight = window->GetHeight();

	LONG xPos = static_cast<LONG>(wWidth * Position.X.Scale) + static_cast<LONG>(Position.X.Offset);
	LONG yPos = static_cast<LONG>(wHeight * Position.Y.Scale) + static_cast<LONG>(Position.Y.Offset);

	LONG width = static_cast<LONG>(textSize.right * Size.X.Scale) + static_cast<LONG>(Size.X.Offset);
	LONG height = static_cast<LONG>(textSize.bottom * Size.Y.Scale) + static_cast<LONG>(Size.Y.Offset);

	LONG xAnchor = (LONG)(width * std::clamp(AnchorPoint.X, 0.0, 1.0));
	LONG yAnchor = (LONG)(height * std::clamp(AnchorPoint.Y, 0.0, 1.0));

	xPos -= xAnchor;
	yPos -= yAnchor;

	AbsolutePosition = { (double)xPos, (double)yPos };
	AbsoluteSize = { (double)width, (double)height };

	float scaleX = static_cast<float>(width) / static_cast<float>(textSize.right);
	float scaleY = static_cast<float>(height) / static_cast<float>(textSize.bottom);

	if (manualDraw) {
		batch->Begin(
			SpriteSortMode_Immediate,
			states->NonPremultiplied(),
			states->PointWrap(),
			nullptr,
			clipRect ? rasterizerState : nullptr,
			[&] {
				if (clipRect) {
					CD3D11_RECT rect(*clipRect);
					context->RSSetScissorRects(1, &rect);
				}
			}
		);
	}

	/*
		_In_ SpriteBatch* spriteBatch, 
		_In_z_ wchar_t const* text, 
		FXMVECTOR position, 
		FXMVECTOR color, 
		float rotation, 
		FXMVECTOR origin, 
		GXMVECTOR scale, 
		SpriteEffects effects = SpriteEffects_None, 
		float layerDepth = 0
	*/

	XMVECTOR position = XMVectorSet((float)xPos, (float)yPos, 0, 0);
	XMVECTOR color = XMVectorSet(TintColor.R, TintColor.G, TintColor.B, 1);
	XMVECTOR origin = XMVectorSet(0, 0, 0, 0);
	XMVECTOR scale = XMVectorSet((float)scaleX, (float)scaleY, 0, 0);
	
	m_font->DrawString(batch, text.c_str(), position, color, 0, origin, scale);

	if (manualDraw) {
		batch->End();
	}
}

Text::~Text() {
	if (m_font) {
		delete m_font;
	}
}
