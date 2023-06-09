#include "Texture2D.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

#include "Renderer.hpp"
#include "MathUtils.hpp"
#include "Win32ErrorHandling.h"
#include "SDLException.hpp"
#include <SDL2/SDL_image.h>

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

Texture2D::Texture2D() {
	TintColor = { 1.0f, 1.0f, 1.0f };

	Rotation = 0;
	Transparency = 0.0f;
	AlphaBlend = false;

	m_actualSize = {};
	m_preAnchoredSize = {};
	m_calculatedSize = {};

	m_sdl_surface = nullptr;
	m_sdl_tex = nullptr;

	m_bDisposeTexture = false;

	Size = UDim2::fromScale(1, 1);
}

Texture2D::Texture2D(std::string fileName) : Texture2D() {
	if (!std::filesystem::exists(fileName)) {
		fileName = std::filesystem::current_path().string() + fileName;
	}

	if (!std::filesystem::exists(fileName)) {
		throw std::runtime_error(fileName + " not found!");
	}

	std::fstream fs(fileName, std::ios::binary | std::ios::in);
	if (!fs.is_open()) {
		throw std::runtime_error(fileName + " cannot opened!");
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
	TintColor = { 1.0f, 1.0f, 1.0f };
	
	LoadImageResources(buffer, size);
}

Texture2D::Texture2D(std::filesystem::path path) : Texture2D() {
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
	TintColor = { 1.0f, 1.0f, 1.0f };

	LoadImageResources(buffer, size);
}

// Do base constructor called before derived constructor?
// https://stackoverflow.com/questions/120547/what-are-the-rules-for-calling-the-superclass-constructor

Texture2D::Texture2D(uint8_t* fileData, size_t size) : Texture2D() {
	uint8_t* buffer = new uint8_t[size];
	memcpy(buffer, fileData, size);

	Rotation = 0;
	Transparency = 0.0f;
	m_actualSize = { 0, 0, 0, 0 };
	m_bDisposeTexture = true;
	TintColor = { 1.0f, 1.0f, 1.0f };

	LoadImageResources(buffer, size);
}

Texture2D::Texture2D(SDL_Texture* texture) : Texture2D() {
	m_bDisposeTexture = false;
	m_sdl_tex = texture;
}

Texture2D::~Texture2D() {
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

void Texture2D::Draw() {
	Draw(true);
}

void Texture2D::Draw(bool manualDraw) {
	Draw(nullptr, manualDraw);
}

void Texture2D::Draw(RECT* clipRect) {
	Draw(clipRect, true);
}

void Texture2D::Draw(RECT* clipRect, bool manualDraw) {
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

void Texture2D::CalculateSize() {
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

RECT Texture2D::GetOriginalRECT() {
	return m_actualSize;
}

Texture2D* Texture2D::FromTexture2D(Texture2D* tex) {
	auto copy = new Texture2D(tex->m_sdl_tex);
	copy->m_actualSize = tex->m_actualSize;
	copy->Position = tex->Position;
	copy->Size = tex->Size;
	
	return copy;
}

Texture2D* Texture2D::FromBMP(uint8_t* fileData, size_t size) {
	return nullptr;
}

Texture2D* Texture2D::FromBMP(std::string fileName) {
	return nullptr;
}

Texture2D* Texture2D::FromJPEG(uint8_t* fileData, size_t size) {
	return nullptr;
}

Texture2D* Texture2D::FromJPEG(std::string fileName) {
	return nullptr;
}

Texture2D* Texture2D::FromPNG(uint8_t* fileData, size_t size) {
	return nullptr;
}

Texture2D* Texture2D::FromPNG(std::string fileName) {
	return nullptr;
}

void Texture2D::LoadImageResources(uint8_t* buffer, size_t size) {
	SDL_RWops* rw = SDL_RWFromMem(buffer, size);
	
	// check if buffer magic is BMP
	if (buffer[0] == 0x42 && buffer[1] == 0x4D) {
		m_sdl_surface = SDL_LoadBMP_RW(rw, 1);
	}
	else {
		m_sdl_surface = IMG_Load_RW(rw, 1);
	}

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
