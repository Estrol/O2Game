#include "Sprite2D.hpp"
#include "Texture2D.hpp"
#include "Window.hpp"

Sprite2D::Sprite2D(std::vector<Texture2D*> textures, float delay) : Sprite2D::Sprite2D() {
	m_textures = textures;
	m_delay = delay;

	Size = UDim2::fromScale(1, 1);
	Position = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };
}

Sprite2D::Sprite2D(std::vector<std::string> textures, float delay) : Sprite2D::Sprite2D() {
	m_delay = delay;
	Size = UDim2::fromScale(1, 1);
	Position = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };

	for (auto& it : textures) {
		m_textures.emplace_back(new Texture2D(it));
	}
}

Sprite2D::Sprite2D(std::vector<std::filesystem::path> textures, float delay) : Sprite2D::Sprite2D() {
	m_delay = delay;
	Size = UDim2::fromScale(1, 1);
	Position = UDim2::fromOffset(0, 0);
	AnchorPoint = { 0, 0 };

	for (auto& it : textures) {
		m_textures.emplace_back(new Texture2D(it));
	}
}

Sprite2D::Sprite2D(std::vector<SDL_Texture*> textures, float delay) : Sprite2D::Sprite2D() {
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

void Sprite2D::Draw(double delta, Rect* rect, bool manual) {
	m_current += delta;
	auto tex = m_textures[m_currentIndex];
	Window* window = Window::GetInstance();

	double xPos = (window->GetBufferWidth() * Position.X.Scale) + (Position.X.Offset);
	double yPos = (window->GetBufferHeight() * Position.Y.Scale) + (Position.Y.Offset);

	double xMPos = (window->GetBufferWidth() * Position2.X.Scale) + (Position2.X.Offset);
	double yMPos = (window->GetBufferHeight() * Position2.Y.Scale) + (Position2.Y.Offset);

	xPos += xMPos;
	yPos += yMPos;

	tex->Position = UDim2::fromOffset(xPos, yPos);
	tex->AlphaBlend = AlphaBlend;
	tex->Size = Size;
	tex->AnchorPoint = AnchorPoint;
	tex->Draw(rect, manual ? false : true);

	if (m_current >= m_delay) {
		m_current = 0;
		
		if (++m_currentIndex >= m_textures.size()) {
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

void Sprite2D::SetFPS(float fps) {
	m_delay = std::clamp(1.0f / fps, 1.0f / 5.0f, FLT_MAX);
}

void Sprite2D::Reset() {
	m_currentIndex = 0;
	m_current = 0;
}
