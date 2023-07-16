#pragma once
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <filesystem>
#include "Vector2.hpp"
#include "UDim2.hpp"
#include "Data/WindowsTypes.hpp"

class Texture2D;

class Sprite2D {
public:
	Sprite2D() = default;

	Sprite2D(std::vector<Texture2D*> textures, float delay = 1.0);
	Sprite2D(std::vector<std::string> textures, float delay = 1.0);
	Sprite2D(std::vector<std::filesystem::path> textures, float delay = 1.0);
	Sprite2D(std::vector<SDL_Texture*> textures, float delay = 1.0);

	~Sprite2D();

	bool AlphaBlend;
	Vector2 AnchorPoint;
	UDim2 Position;
	UDim2 Position2;
	UDim2 Size;

	void Draw(double delta, bool manual = false);
	void Draw(double delta, Rect* rect, bool manual = false);

	Texture2D* GetTexture();
	void SetFPS(float fps);
	void Reset();

private:
	double m_delay = 1.0;
	double m_current = 0.0;
	float m_currentTime = 0;
	int m_currentIndex = 0;

	std::vector<Texture2D*> m_textures;
};
