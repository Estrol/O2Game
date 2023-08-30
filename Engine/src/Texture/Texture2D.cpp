#include "Texture/Texture2D.h"
#include <filesystem>
#include <fstream>
#include <iostream>

#include "Rendering/Renderer.h"
#include "Texture/MathUtils.h"
#include "Exception/SDLException.h"
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "Rendering/Vulkan/VulkanEngine.h"
#include "Rendering/Vulkan/Texture2DVulkan.h"
#include "../Rendering/Vulkan/Texture2DVulkan_Internal.h"
#include "../Data/Imgui/imgui_impl_vulkan.h"

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

	m_ready = true;
}

Texture2D::Texture2D(Texture2D_Vulkan* texture) : Texture2D() {
	m_bDisposeTexture = false;
	m_vk_tex = texture;

	m_ready = true;
}

Texture2D::~Texture2D() {
	if (m_bDisposeTexture) {
		if (m_vk_tex) {
			auto vk_tex = m_vk_tex;

			vkTexture::ReleaseTexture(vk_tex);

			m_vk_tex = nullptr;
		} else {
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
}

void Texture2D::Draw() {
	Draw(true);
}

void Texture2D::Draw(bool manualDraw) {
	Draw(nullptr, manualDraw);
}

void Texture2D::Draw(Rect* clipRect) {
	Draw(clipRect, true);
}

void Texture2D::Draw(Rect* clipRect, bool manualDraw) {
	Renderer* renderer = Renderer::GetInstance();
	auto window = GameWindow::GetInstance();
	bool scaleOutput = window->IsScaleOutput();
	CalculateSize();

	if (!m_ready) return;

	if (renderer->IsVulkan() && m_vk_tex) {
		auto vulkan_driver = renderer->GetVulkanEngine();
		auto cmd = vulkan_driver->get_current_frame()._mainCommandBuffer;

		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = window->GetWidth();
		scissor.extent.height = window->GetHeight();

		if (clipRect) {
			scissor.offset.x = clipRect->left;
			scissor.offset.y = clipRect->top;
			scissor.extent.width = clipRect->right - clipRect->left;
			scissor.extent.height = clipRect->bottom - clipRect->top;
		}

		VkDescriptorSet imageId = m_vk_tex->DS;

		std::vector<ImDrawVert> vertexData(6);
		float x1 = m_calculatedSizeF.left;
		float y1 = m_calculatedSizeF.top;
		float x2 = m_calculatedSizeF.left + m_calculatedSizeF.right;
		float y2 = m_calculatedSizeF.top + m_calculatedSizeF.bottom;

		if (scaleOutput) {
			if (clipRect) {
				scissor.offset.x = static_cast<int32_t>(clipRect->left * window->GetWidthScale());
				scissor.offset.y = static_cast<int32_t>(clipRect->top * window->GetHeightScale());
				scissor.extent.width = static_cast<int32_t>((clipRect->right - clipRect->left) * window->GetWidthScale());
				scissor.extent.height = static_cast<int32_t>((clipRect->bottom - clipRect->top) * window->GetHeightScale());
			}

			x1 *= window->GetWidthScale();
			y1 *= window->GetHeightScale();
			x2 *= window->GetWidthScale();
			y2 *= window->GetHeightScale();
		}

		if (x2 <= 0 || y2 <= 0) {
			return;
		}

		ImVec2 uv1(0.0f, 0.0f);  // Top-left UV coordinate
		ImVec2 uv2(1.0f, 0.0f);  // Top-right UV coordinate
		ImVec2 uv3(1.0f, 1.0f);  // Bottom-right UV coordinate
		ImVec2 uv4(0.0f, 1.0f);  // Bottom-left UV coordinate

		ImU32 color = IM_COL32_WHITE;

		ImDrawVert vertex1;
		vertex1.pos = ImVec2(x1, y1);
		vertex1.uv = uv1;
		vertex1.col = color;

		ImDrawVert vertex2;
		vertex2.pos = ImVec2(x2, y1);
		vertex2.uv = uv2;
		vertex2.col = color;

		ImDrawVert vertex3;
		vertex3.pos = ImVec2(x2, y2);
		vertex3.uv = uv3;
		vertex3.col = color;

		ImDrawVert vertex4;
		vertex4.pos = ImVec2(x1, y1);
		vertex4.uv = uv1;
		vertex4.col = color;

		ImDrawVert vertex5;
		vertex5.pos = ImVec2(x2, y2);
		vertex5.uv = uv3;
		vertex5.col = color;

		ImDrawVert vertex6;
		vertex6.pos = ImVec2(x1, y2);
		vertex6.uv = uv4;
		vertex6.col = color;

		vertexData[0] = vertex1;
		vertexData[1] = vertex2;
		vertexData[2] = vertex3;
		vertexData[3] = vertex4;
		vertexData[4] = vertex5;
		vertexData[5] = vertex6;

		std::vector<uint16_t> indicies = { 0, 1, 2, 3, 4, 5 };

		SubmitQueueInfo info = {};
		info.AlphaBlend = AlphaBlend;
		info.descriptor = imageId;
		info.vertices = vertexData;
		info.indices = indicies;
		info.scissor = scissor;

		// submit to queue
		vulkan_driver->queue_submit(info);
	}
	else {
		SDL_FRect destRect = { m_calculatedSizeF.left, m_calculatedSizeF.top, m_calculatedSizeF.right, m_calculatedSizeF.bottom };
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
				testClip.x = static_cast<int>(testClip.x * window->GetWidthScale());
				testClip.y = static_cast<int>(testClip.y * window->GetHeightScale());
				testClip.w = static_cast<int>(testClip.w * window->GetWidthScale());
				testClip.h = static_cast<int>(testClip.h * window->GetHeightScale());
			}

			SDL_RenderSetClipRect(renderer->GetSDLRenderer(), &testClip);
		}

		if (AlphaBlend) {
			SDL_GetTextureBlendMode(m_sdl_tex, &oldBlendMode);
			SDL_SetTextureBlendMode(m_sdl_tex, renderer->GetSDLBlendMode());
		}

		SDL_SetTextureColorMod(m_sdl_tex, static_cast<uint8_t>(TintColor.R * 255), static_cast<uint8_t>(TintColor.G * 255), static_cast<uint8_t>(TintColor.B * 255));
		SDL_SetTextureAlphaMod(m_sdl_tex, static_cast<uint8_t>(255 - (Transparency / 100.0) * 255));

		int error = SDL_RenderCopyExF(
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
}

void Texture2D::CalculateSize() {
	GameWindow* window = GameWindow::GetInstance();
	int wWidth = window->GetBufferWidth();
	int wHeight = window->GetBufferHeight();

	float xPos = static_cast<float>((wWidth * Position.X.Scale) + Position.X.Offset);
	float yPos = static_cast<float>((wHeight * Position.Y.Scale) + Position.Y.Offset);
	
	float width = static_cast<float>((m_actualSize.right * Size.X.Scale) + Size.X.Offset);
	float height = static_cast<float>((m_actualSize.bottom * Size.Y.Scale) + Size.Y.Offset);

	m_preAnchoredSize = { (LONG)xPos, (LONG)yPos, (LONG)width, (LONG)height };
	m_preAnchoredSizeF = { xPos, yPos, width, height };

	float xAnchor = width * std::clamp((float)AnchorPoint.X, 0.0f, 1.0f);
	float yAnchor = height * std::clamp((float)AnchorPoint.Y, 0.0f, 1.0f);
	
	xPos -= xAnchor;
	yPos -= yAnchor;

	m_calculatedSize = { (LONG)xPos, (LONG)yPos, (LONG)width, (LONG)height };
	m_calculatedSizeF = { xPos, yPos, width, height };

	AbsolutePosition = { xPos, yPos };
	AbsoluteSize = { width, height };
}

Rect Texture2D::GetOriginalRECT() {
	return m_actualSize;
}

void Texture2D::SetOriginalRECT(Rect size) {
	m_actualSize = size;
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
	if (Renderer::GetInstance()->IsVulkan()) {
		auto tex_data = vkTexture::TexLoadImage(buffer, size);

		m_actualSize = { 0, 0, tex_data->Width, tex_data->Height };
		m_vk_tex = tex_data;

		m_bDisposeTexture = true;
		m_ready = true;
		delete[] buffer;
	}
	else {
		SDL_RWops* rw = SDL_RWFromMem(buffer, (int)size);

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

		m_ready = true;
	}
}
