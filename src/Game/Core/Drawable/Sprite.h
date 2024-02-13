/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include <Graphics/Utils/Rect.h>
#include <Math/Color3.h>
#include <UI/Sprite.h>
#include <filesystem>

class Sprite : public UI::Sprite
{
public:
    Sprite();
    Sprite(
        std::filesystem::path               path,
        std::vector<std::vector<glm::vec2>> texCoords,
        float                               fps);
    Sprite(
        std::shared_ptr<Graphics::Texture2D> texture,
        std::vector<std::vector<glm::vec2>>  texCoords,
        float                                fps);

    virtual void Draw() override;
    virtual void Draw(Rect rect) override;
    virtual void Draw(double delta);
    virtual void Draw(double delta, Rect rect);
    virtual void Update(double fixedDelta);

    void SetIndexAt(int index);
    void SetDelay(float fps);
    void SetRepeatIndex(int index);
    void Reset();
    void ResetLast();

    bool Repeat;
    bool UpdateOnDraw;

    UDim2 Position2;
    bool  AlphaBlend;

    void CalculateSize() override;

protected:
    int m_RepeatIndex;
};