#include "Texture2D.hpp"
#include <filesystem>
#include <fstream>

#include <directxtk/WICTextureLoader.h>
#include "Renderer.hpp"
#include "MathUtils.hpp"

using namespace DirectX;

Texture2D::Texture2D() {
	m_pTexture = nullptr;

	Rotation = 0;
	Transparency = 0.0f;
	m_actualSize = { 0, 0, 0, 0 };
	m_bDisposeTexture = true;
}

Texture2D::Texture2D(std::string fileName) {
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
	
	LoadImageResources(buffer, size);
}

Texture2D::Texture2D(uint8_t* fileData, size_t size) {
	uint8_t* buffer = new uint8_t[size];
	memcpy(buffer, fileData, size);

	Rotation = 0;
	Transparency = 0.0f;
	m_actualSize = { 0, 0, 0, 0 };
	m_bDisposeTexture = true;

	LoadImageResources(buffer, size);
}

Texture2D::Texture2D(ID3D11ShaderResourceView* texture) {
	m_pTexture = texture;

	Rotation = 0;
	Transparency = 0.0f;
	m_actualSize = { 0, 0, 0, 0 };
	m_bDisposeTexture = false;
}

Texture2D::~Texture2D() {
	if (m_bDisposeTexture) {
		if (m_pTexture) {
			m_pTexture->Release();
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
	auto batch = m_pSpriteBatch != nullptr ? m_pSpriteBatch : renderer->GetSpriteBatch();
	auto states = renderer->GetStates();
	auto context = renderer->GetImmediateContext();
	auto rasterizerState = renderer->GetRasterizerState();

	CalculateSize();

	float scaleX = static_cast<float>(m_preAnchoredSize.right) / static_cast<float>(m_actualSize.right);
	float scaleY = static_cast<float>(m_preAnchoredSize.bottom) / static_cast<float>(m_actualSize.bottom);

	if (manualDraw) {
		batch->Begin(
			SpriteSortMode_Deferred,
			states->NonPremultiplied(),
			nullptr,
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

	XMVECTOR position = { m_calculatedSize.left, m_calculatedSize.top, 0, 0 };
	XMVECTOR origin = { 0.5, 0.5, 0, 0 };
	XMVECTOR scale = { scaleX, scaleY, 1, 1 };

	try {
		batch->Draw(
			m_pTexture,
			position,
			&m_actualSize,
			Colors::White,
			Rotation,
			origin,
			scale
		);
	}
	catch (std::logic_error& error) {
		if (error.what() == "Begin must be called before Draw") {
			MessageBoxA(NULL, "Texture2D::Draw missing SpriteBatch::Begin", "EstEngine, Error", MB_ICONERROR);
			return;
		}
	}

	if (manualDraw) {
		batch->End();
	}
}

void Texture2D::CalculateSize() {
	Window* window = Window::GetInstance();
	int wWidth = window->GetWidth();
	int wHeight = window->GetHeight();

	LONG xPos = (wWidth * Position.X.Scale) + Position.X.Offset;
	LONG yPos = (wHeight * Position.Y.Scale) + Position.Y.Offset;

	LONG width = (m_actualSize.right * Size.X.Scale) + Size.X.Offset;
	LONG height = (m_actualSize.bottom * Size.Y.Scale) + Size.Y.Offset;

	m_preAnchoredSize = { xPos, yPos, width, height };

	LONG xAnchor = (LONG)(width * std::clamp(AnchorPoint.X, 0.0, 1.0));
	LONG yAnchor = (LONG)(height * std::clamp(AnchorPoint.Y, 0.0, 1.0));
	
	xPos -= xAnchor;
	yPos -= yAnchor;

	m_calculatedSize = { xPos, yPos, width, height };

	AbsolutePosition = { (double)xPos, (double)yPos };
	AbsoluteSize = { (double)width, (double)height };
}

Texture2D* Texture2D::FromTexture2D(Texture2D* tex) {
	//return new Texture2D(tex->m_pBuffer, tex->m_bufferSize);
	auto copy = new Texture2D(tex->m_pTexture);
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
	ID3D11Device* device = Renderer::GetInstance()->GetDevice();
	ID3D11DeviceContext* context = Renderer::GetInstance()->GetImmediateContext();
	
	ID3D11Resource* resource = nullptr;
	HRESULT hr = CreateWICTextureFromMemoryEx(
		device,
		context,
		buffer,
		size,
		0,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		0,
		0,
		WIC_LOADER_FORCE_RGBA32 | WIC_LOADER_IGNORE_SRGB,
		&resource,
		&m_pTexture
	);

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to load texture resource view!");
	}

	ID3D11Texture2D* texture = nullptr;
	hr = resource->QueryInterface<ID3D11Texture2D>(&texture);

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to load texture resource view!");
	}

	D3D11_TEXTURE2D_DESC desc;
	texture->GetDesc(&desc);

	Size.X.Scale = 1.0f;
	Size.X.Offset = 0;
	Size.Y.Scale = 1.0f;
	Size.Y.Offset = 0;
	
	m_actualSize = { 0, 0, (LONG)desc.Width, (LONG)desc.Height };

	// Free the copied buffer
	delete[] buffer;

	texture->Release();
	resource->Release();
}
