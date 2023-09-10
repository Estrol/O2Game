#pragma once
#include <string>
#include <SDL2/SDL.h>
#include <filesystem>

#include "../Rendering/WindowsTypes.h"
#include "Rendering/Vulkan/VulkanEngine.h"
#include "Texture/Bitmap.h"
#include "UDim2.h"
#include "Vector2.h"
#include "Color3.h"

struct Texture2D_Vulkan;

class Texture2D {
#if _DEBUG
	const char SIGNATURE[25] = "Texture2D";
#endif

public:
	Texture2D();
	Texture2D(std::string fileName);
	Texture2D(std::filesystem::path path);
	Texture2D(uint8_t* fileData, size_t size);
	Texture2D(SDL_Texture* texture);
	Texture2D(Texture2D_Vulkan* texture);
	~Texture2D();

	void Draw();
	void Draw(bool manualDraw);
	void Draw(Rect* clipRect);
	void Draw(Rect* clipRect, bool manualDraw);

	void CalculateSize();

	float Transparency;
	float Rotation;
	bool AlphaBlend;
	bool m_ready = false;

	UDim2 Size;
	UDim2 Position;
	UDim2 Position2;
	Color3 TintColor;

	Vector2 AnchorPoint;
	Vector2 AbsoluteSize;
	Vector2 AbsolutePosition;

	Rect GetOriginalRECT();
	void SetOriginalRECT(Rect size);

	static Texture2D* FromTexture2D(Texture2D* tex);

	static Texture2D* FromBMP(uint8_t* fileData, size_t size);
	static Texture2D* FromBMP(std::string fileName);
	static Texture2D* FromJPEG(uint8_t* fileData, size_t size);
	static Texture2D* FromJPEG(std::string fileName);
	static Texture2D* FromPNG(uint8_t* fileData, size_t size);
	static Texture2D* FromPNG(std::string fileName);
	
protected:
	void LoadImageResources(uint8_t* buffer, size_t size);
	bool m_bDisposeTexture;
	
	Rect m_calculatedSize;
	Rect m_preAnchoredSize;
	RectF m_calculatedSizeF;
	RectF m_preAnchoredSizeF;

	Rect m_actualSize;
	SDL_Texture* m_sdl_tex;
	SDL_Surface* m_sdl_surface;

	Texture2D_Vulkan* m_vk_tex;
};