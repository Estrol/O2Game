#include "Texture2D.hpp"
#include <filesystem>
#include <fstream>

#include <directxtk/WICTextureLoader.h>
#include "Renderer.hpp"
#include "MathUtils.hpp"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

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
		SAFE_RELEASE(m_pTexture)
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

				if (AlphaBlend) {
					ID3D11SamplerState* samplerState = nullptr;

					CD3D11_SAMPLER_DESC samplerDesc = {};
					ZeroMemory(&samplerDesc, sizeof(samplerDesc));
					samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.MipLODBias = 0.0f;
					samplerDesc.MaxAnisotropy = 1;
					samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
					samplerDesc.BorderColor[0] = 0;
					samplerDesc.BorderColor[1] = 0;
					samplerDesc.BorderColor[2] = 0;
					samplerDesc.BorderColor[3] = 0;
					samplerDesc.MinLOD = 0;
					samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

					renderer->GetDevice()->CreateSamplerState(&samplerDesc, &samplerState);

					context->PSSetSamplers(0, 1, &samplerState);

					SAFE_RELEASE(samplerState)
				}
			}
		);
	}

	XMVECTOR position = { (float)m_calculatedSize.left, (float)m_calculatedSize.top, 0, 0 };
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
