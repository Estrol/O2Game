#pragma once
#include <vector>
#include <string>
#include <d3d11.h>
#include <filesystem>
#include "Vector2.hpp"
#include "UDim2.hpp"
class Texture2D;

class Sprite2D {
public:
	Sprite2D() = default;

	Sprite2D(std::vector<Texture2D*> textures, float delay = 1.0);
	Sprite2D(std::vector<std::string> textures, float delay = 1.0);
	Sprite2D(std::vector<std::filesystem::path> textures, float delay = 1.0);
	Sprite2D(std::vector<ID3D11ShaderResourceView*> textures, float delay = 1.0);

	~Sprite2D();

	bool AlphaBlend;
	Vector2 AnchorPoint;
	UDim2 Position;
	UDim2 Position2;
	UDim2 Size;

	void Draw(double delta, bool manual = false);
	void Draw(double delta, RECT* rect, bool manual = false);

	Texture2D* GetTexture();
	void SetDelay(double delay);
	void Reset();

private:
	double m_delay = 1.0;
	double m_current = 0.0;
	int m_currentIndex = 0;

	std::vector<Texture2D*> m_textures;
};

