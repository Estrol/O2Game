#pragma once
#include <d3d11.h>
#include <string>
#include <SDL2/SDL.h>
#include <filesystem>

#include "UDim2.hpp"
#include "Vector2.hpp"
#include "Color3.hpp"

struct RECT_F {
	float left;
	float top;
	float right;
	float bottom;
};

class Texture2D {
public:
	Texture2D();
	Texture2D(std::string fileName);
	Texture2D(std::filesystem::path path);
	Texture2D(uint8_t* fileData, size_t size);
	Texture2D(SDL_Texture* texture);
	~Texture2D();

	void Draw();
	void Draw(bool manualDraw);
	void Draw(RECT* clipRect);
	void Draw(RECT* clipRect, bool manualDraw);

	void CalculateSize();

	float Transparency;
	float Rotation;
	bool AlphaBlend;

	UDim2 Size;
	UDim2 Position;
	UDim2 Position2;
	Color3 TintColor;

	Vector2 AnchorPoint;
	Vector2 AbsoluteSize;
	Vector2 AbsolutePosition;

	RECT GetOriginalRECT();
	void SetOriginalRECT(RECT size);

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
	
	RECT m_calculatedSize;
	RECT m_preAnchoredSize;
	RECT_F m_calculatedSizeF;
	RECT_F m_preAnchoredSizeF;

	RECT m_actualSize;
	SDL_Texture* m_sdl_tex;
	SDL_Surface* m_sdl_surface;
};