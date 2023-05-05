#include "Sprite2D.hpp"
#include "Texture2D.hpp"
#include "Window.hpp"

Sprite2D::Sprite2D(std::vector<Texture2D*> textures, float delay) {
	m_textures = textures;
	m_delay = delay;

	Size = UDim2::fromScale(1, 1);
	Position = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };
}

Sprite2D::Sprite2D(std::vector<std::string> textures, float delay) {
	m_delay = delay;
	Size = UDim2::fromScale(1, 1);
	Position = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };

	for (auto& it : textures) {
		m_textures.emplace_back(new Texture2D(it));
	}
}

Sprite2D::Sprite2D(std::vector<std::filesystem::path> textures, float delay) {
	m_delay = delay;
	Size = UDim2::fromScale(1, 1);
	Position = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };

	for (auto& it : textures) {
		m_textures.emplace_back(new Texture2D(it));
	}
}

Sprite2D::Sprite2D(std::vector<ID3D11ShaderResourceView*> textures, float delay) {
	m_delay = delay;
	Size = UDim2::fromScale(1, 1);
	Position = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };

	for (auto& it : textures) {
		m_textures.emplace_back(new Texture2D(it));
	}
}

Sprite2D::~Sprite2D() {
	for (auto& it : m_textures) {
		delete it;
	}
}

void Sprite2D::Draw(double delta, bool manual) {
	Draw(delta, nullptr, manual);
}

void Sprite2D::Draw(double delta, RECT* rect, bool manual) {
	m_current += delta;
	auto tex = m_textures[m_currentIndex];
	Window* window = Window::GetInstance();

	LONG xPos = static_cast<LONG>(window->GetBufferWidth() * Position.X.Scale) + static_cast<LONG>(Position.X.Offset);
	LONG yPos = static_cast<LONG>(window->GetBufferHeight() * Position.Y.Scale) + static_cast<LONG>(Position.Y.Offset);

	LONG xMPos = static_cast<LONG>(window->GetBufferWidth() * Position2.X.Scale) + static_cast<LONG>(Position2.X.Offset);
	LONG yMPos = static_cast<LONG>(window->GetBufferHeight() * Position2.Y.Scale) + static_cast<LONG>(Position2.Y.Offset);

	xPos += xMPos;
	yPos += yMPos;

	tex->Position = UDim2::fromOffset(xPos, yPos);
	tex->AlphaBlend = AlphaBlend;
	tex->Size = Size;
	tex->AnchorPoint = AnchorPoint;
	tex->Draw(rect, manual ? false : true);

	if (m_current >= m_delay) {
		m_current = 0;
		m_currentIndex += 1;

		if (m_currentIndex >= m_textures.size()) {
			m_currentIndex = 0;
		}
	}
}

Texture2D* Sprite2D::GetTexture() {
	auto tex = m_textures[m_currentIndex];
	tex->Position = Position;
	tex->Size = Size;
	tex->AnchorPoint = AnchorPoint;

	return tex;
}

void Sprite2D::SetDelay(double delay) {
	m_delay = delay;
}

void Sprite2D::Reset() {
	m_currentIndex = 0;
	m_current = 0;
}
