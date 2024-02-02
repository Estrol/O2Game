/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "Image.h"
#include <filesystem>

class Sprite
{
public:
    Sprite();
    Sprite(std::vector<std::filesystem::path> paths, float fps = 0.0f);
    Sprite(std::vector<Image *> paths, float fps = 0.0f);
    Sprite(std::vector<std::filesystem::path> paths, int repeatIndex, float fps);
    Sprite(std::vector<Image *> paths, int repeatIndex, float fps);

    ~Sprite();

    void SetIndexAt(int index);
    void SetDelay(float fps);
    void SetRepeatIndex(int index);
    void Reset();
    void ResetLast();

    bool AlphaBlend;   // Enable alpha blending
    bool Repeat;       // Whatever the sprite should repeat or not
    bool UpdateOnDraw; // Whatever the sprite index should update on draw or not, can be useful to update in fixed update thread

    Vector2 AnchorPoint; // Anchor point of the sprite
    Color3  TintColor;   // Tint color of the sprite
    UDim2   Position;
    UDim2   Position2;
    UDim2   Size;

    Vector2 AbsoluteSize;
    Vector2 AbsolutePosition;

    void Update(double fixedDelta);

    void Draw(double delta);
    void Draw(double delta, Rect rect);

    Image **GetImages();
    void    CalculateSize();

private:
    uint32_t m_CurrentIndex;
    uint32_t m_RepeatIndex;
    uint32_t m_MaxIndex;
    float    m_Delay;
    float    m_CurrentTime;
    bool     m_ShouldDispose;

    std::vector<Image *> m_Images;
};