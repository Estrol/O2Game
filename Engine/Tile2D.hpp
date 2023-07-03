#pragma once
#include <string>
#include <filesystem>
#include <SDL2/SDL.h>

#include "UDim2.hpp"
#include "Vector2.hpp"
#include "Color3.hpp"

class Tile2D {
public:
	Tile2D();
	Tile2D(std::string fileName);
	Tile2D(std::filesystem::path path);
	Tile2D(uint8_t* fileData, size_t size);
	//Tile2D(ID3D11ShaderResourceView* texture);
	~Tile2D();

	void Draw();
	void Draw(bool manualDraw);
	void Draw(RECT* clipRect);
	void Draw(RECT* clipRect, bool manualDraw);

	void CalculateSize();

	float Transparency;
	float Rotation;

	UDim2 Size;
	UDim2 Position;
	Color3 TintColor;
	bool AlphaBlend;

	Vector2 AnchorPoint;
	Vector2 AbsoluteSize;
	Vector2 AbsolutePosition;

protected:
	void LoadImageResources(uint8_t* buffer, size_t size);
	bool m_bDisposeTexture;

	RECT m_actualSize;
	RECT m_calculatedSize;
	RECT m_preAnchoredSize;

	SDL_Texture* m_sdl_tex;
	SDL_Surface* m_sdl_surface;

	/*ID3D11ShaderResourceView* m_pTexture;
	DirectX::SpriteBatch* m_pSpriteBatch = nullptr;*/
};

