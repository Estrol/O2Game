#include "Tile2D.hpp"
#include <filesystem>
#include <fstream>

#include <SDL2/SDL_image.h>
#include "Renderer.hpp"
#include "MathUtils.hpp"
#include "SDLException.hpp"

Tile2D::Tile2D() {
	Rotation = 0;
	Transparency = 0.0f;
	m_actualSize = { 0, 0, 0, 0 };
	m_bDisposeTexture = true;

	AlphaBlend = false;
	Size = UDim2::fromScale(1, 1);
	Position = UDim2::fromOffset(0, 0);

	TintColor = { 1.0, 1.0, 1.0 };
}

Tile2D::Tile2D(std::string fileName) : Tile2D() {
	auto path = std::filesystem::current_path().string();

	if (!fileName.starts_with(path)) {
		fileName = path + fileName;
	}

	if (!std::filesystem::exists(fileName)) {
		throw std::runtime_error(fileName + " not found!");
	}

	std::fstream fs(fileName, std::ios::binary | std::ios::in);
	if (!fs.is_open()) {
		throw std::runtime_error(fileName + " cannot opened!");
	}

	fs.seekg(0, std::ios::end);
	int size = fs.tellg();
	fs.seekg(0, std::ios::beg);

	uint8_t* buffer = new uint8_t[size];
	fs.read((char*)buffer, size);
	fs.close();

	Rotation = 0;
	Transparency = 0.0f;
	m_actualSize = { 0, 0, 0, 0 };
	m_bDisposeTexture = true;
	TintColor = { 1.0, 1.0, 1.0 };

	LoadImageResources(buffer, size);
}

Tile2D::Tile2D(std::filesystem::path path) : Tile2D() {
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error(path.string() + " not found!");
	}

	std::fstream fs(path, std::ios::binary | std::ios::in);
	if (!fs.is_open()) {
		throw std::runtime_error(path.string() + " cannot opened!");
	}

	fs.seekg(0, std::ios::end);
	size_t size = fs.tellg();
	fs.seekg(0, std::ios::beg);

	uint8_t* buffer = new uint8_t[size];
	fs.read((char*)buffer, size);
	fs.close();

	Rotation = 0;
	Transparency = 0.0f;
	m_actualSize = { 0, 0, 0, 0 };
	m_bDisposeTexture = true;
	AlphaBlend = false;
	TintColor = { 1.0f, 1.0f, 1.0f };

	LoadImageResources(buffer, size);
}

Tile2D::Tile2D(uint8_t* fileData, size_t size) : Tile2D() {
	uint8_t* buffer = new uint8_t[size];
	memcpy(buffer, fileData, size);

	Rotation = 0;
	Transparency = 0.0f;
	m_actualSize = { 0, 0, 0, 0 };
	m_bDisposeTexture = true;
	AlphaBlend = false;
	TintColor = { 1.0, 1.0, 1.0 };

	LoadImageResources(buffer, size);
}

//Tile2D::Tile2D(ID3D11ShaderResourceView* texture) {
//	m_pTexture = texture;
//
//	Rotation = 0;
//	Transparency = 0.0f;
//	m_actualSize = { 0, 0, 0, 0 };
//	m_bDisposeTexture = false;
//	AlphaBlend = false;
//	TintColor = { 1.0, 1.0, 1.0 };
//}

Tile2D::~Tile2D() {
	if (m_bDisposeTexture) {
		if (m_sdl_tex) {
			SDL_DestroyTexture(m_sdl_tex);
			m_sdl_tex = nullptr;
		}

		if (m_sdl_surface) {
			SDL_FreeSurface(m_sdl_surface);
			m_sdl_surface = nullptr;
		}
	}
}

void Tile2D::Draw() {
	Draw(nullptr, true);
}

void Tile2D::Draw(bool manualDraw) {
	Draw(nullptr, manualDraw);
}

void Tile2D::Draw(RECT* clipRect) {
	Draw(clipRect, true);
}

void Tile2D::Draw(RECT* clipRect, bool manualDraw) {
	Renderer* renderer = Renderer::GetInstance();

	CalculateSize();

	float scaleX = static_cast<float>(m_preAnchoredSize.right) / static_cast<float>(m_actualSize.right);
	float scaleY = static_cast<float>(m_preAnchoredSize.bottom) / static_cast<float>(m_actualSize.bottom);

	auto window = Window::GetInstance();

	bool scaleOutput = window->IsScaleOutput();

	SDL_Rect destRect = { m_calculatedSize.left, m_calculatedSize.top, m_calculatedSize.right, m_calculatedSize.bottom };
	if (scaleOutput) {
		destRect.x = destRect.x * window->GetWidthScale();
		destRect.y = destRect.y * window->GetHeightScale();
		destRect.w = destRect.w * window->GetWidthScale();
		destRect.h = destRect.h * window->GetHeightScale();
	}

	SDL_Rect originClip = {};
	SDL_BlendMode oldBlendMode = SDL_BLENDMODE_NONE;

	if (clipRect) {
		SDL_RenderGetClipRect(renderer->GetSDLRenderer(), &originClip);

		SDL_Rect testClip = { clipRect->left, clipRect->top, clipRect->right - clipRect->left, clipRect->bottom - clipRect->top };
		if (scaleOutput) {
			testClip.x = testClip.x * window->GetWidthScale();
			testClip.y = testClip.y * window->GetHeightScale();
			testClip.w = testClip.w * window->GetWidthScale();
			testClip.h = testClip.h * window->GetHeightScale();
		}

		SDL_RenderSetClipRect(renderer->GetSDLRenderer(), &testClip);
	}

	if (AlphaBlend) {
		SDL_GetTextureBlendMode(m_sdl_tex, &oldBlendMode);
		SDL_SetTextureBlendMode(m_sdl_tex, renderer->GetSDLBlendMode());
	}

	SDL_SetTextureColorMod(m_sdl_tex, static_cast<uint8_t>(TintColor.R * 255), static_cast<uint8_t>(TintColor.G * 255), static_cast<uint8_t>(TintColor.B * 255));
	SDL_SetTextureAlphaMod(m_sdl_tex, static_cast<uint8_t>(255 - (Transparency / 100.0) * 255));

	int error = SDL_RenderCopyEx(
		renderer->GetSDLRenderer(),
		m_sdl_tex,
		nullptr,
		&destRect,
		Rotation,
		nullptr,
		(SDL_RendererFlip)0
	);

	if (error != 0) {
		throw SDLException();
	}

	if (AlphaBlend) {
		SDL_SetTextureBlendMode(m_sdl_tex, oldBlendMode);
	}

	if (clipRect) {
		if (originClip.w == 0 || originClip.h == 0) {
			SDL_RenderSetClipRect(renderer->GetSDLRenderer(), nullptr);
		}
		else {
			SDL_RenderSetClipRect(renderer->GetSDLRenderer(), &originClip);
		}
	}
}

void Tile2D::CalculateSize() {
	Window* window = Window::GetInstance();
	int wWidth = window->GetWidth();
	int wHeight = window->GetHeight();

	LONG xPos = static_cast<LONG>(wWidth * Position.X.Scale) + static_cast<LONG>(Position.X.Offset);
	LONG yPos = static_cast<LONG>(wHeight * Position.Y.Scale) + static_cast<LONG>(Position.Y.Offset);

	LONG width = static_cast<LONG>(m_actualSize.right * Size.X.Scale) + static_cast<LONG>(Size.X.Offset);
	LONG height = static_cast<LONG>(m_actualSize.bottom * Size.Y.Scale) + static_cast<LONG>(Size.Y.Offset);

	m_preAnchoredSize = { xPos, yPos, width, height };

	LONG xAnchor = (LONG)(width * std::clamp(AnchorPoint.X, 0.0, 1.0));
	LONG yAnchor = (LONG)(height * std::clamp(AnchorPoint.Y, 0.0, 1.0));

	xPos -= xAnchor;
	yPos -= yAnchor;

	m_calculatedSize = { xPos, yPos, width, height };

	AbsolutePosition = { (double)xPos, (double)yPos };
	AbsoluteSize = { (double)width, (double)height };
}

void Tile2D::LoadImageResources(uint8_t* buffer, size_t size) {
	SDL_RWops* rw = SDL_RWFromMem(buffer, size);
	m_sdl_surface = IMG_Load_RW(rw, 1);
	if (!m_sdl_surface) {
		throw SDLException();
	}

	m_sdl_tex = SDL_CreateTextureFromSurface(Renderer::GetInstance()->GetSDLRenderer(), m_sdl_surface);
	if (!m_sdl_tex) {
		throw SDLException();
	}
	
	// sdl get texture resolution
	int w, h;
	SDL_QueryTexture(m_sdl_tex, nullptr, nullptr, &w, &h);

	m_bDisposeTexture = true;
	delete[] buffer;
	m_actualSize = { 0, 0, w, h };
}
