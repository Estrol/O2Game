#pragma once
#include <d3d11.h>
#include <string>
#include <directxtk/SpriteBatch.h>

#include "UDim2.hpp"
#include "Vector2.hpp"

class Texture2D {
public:
	Texture2D();
	Texture2D(std::string fileName);
	Texture2D(uint8_t* fileData, size_t size);
	Texture2D(ID3D11ShaderResourceView* texture);
	~Texture2D();

	void Draw();
	void Draw(bool manualDraw);
	void Draw(RECT* clipRect);
	void Draw(RECT* clipRect, bool manualDraw);

	void CalculateSize();

	float Transparency;
	float Rotation;

	UDim2 Size;
	UDim2 Position;

	Vector2 AnchorPoint;
	Vector2 AbsoluteSize;
	Vector2 AbsolutePosition;

	RECT GetOriginalRECT();

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

	RECT m_actualSize;
	ID3D11ShaderResourceView* m_pTexture;
	DirectX::SpriteBatch* m_pSpriteBatch = nullptr;
};