#pragma once
#include "Rendering/WindowsTypes.h"
#include "UDim2.h"
#include "Vector2.h"
#include <SDL2/SDL.h>
#include <filesystem>
#include <string>
#include <vector>

class Texture2D;

class Sprite2D
{
#if _DEBUG
    const char SIGNATURE[25] = "Sprite2D";
#endif

public:
    Sprite2D() = default;

    Sprite2D(std::vector<Texture2D *> textures, float delay = 1.0f);
    Sprite2D(std::vector<std::string> textures, float delay = 1.0f);
    Sprite2D(std::vector<std::filesystem::path> textures, float delay = 1.0f);
    Sprite2D(std::vector<SDL_Texture *> textures, float delay = 1.0f);

    ~Sprite2D();

    bool    AlphaBlend;
    Vector2 AnchorPoint;
    UDim2   Position;
    UDim2   Position2;
    UDim2   Size;

    void Draw(double delta, bool manual = false);
    void Draw(double delta, Rect *rect, bool manual = false);
    void DrawStop(double delta, bool manual = false);
    void DrawOnce(double delta, bool manual = false);

    Texture2D *GetTexture();
    void       SetFPS(float fps);
    void       Reset();

private:
    double m_spritespeed = 1.0;
    float  m_currentTime = 0;
    int    m_currentIndex = 0;

    bool m_drawOnce = false;

    std::vector<Texture2D *> m_textures;
};
