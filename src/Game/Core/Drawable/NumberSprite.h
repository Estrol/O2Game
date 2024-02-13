/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "Sprite.h"
#include <map>
#include <string>
#include <vector>

enum class NumericPosition {
    LEFT,
    MID,
    RIGHT
};

NumericPosition IntToPos(int i);

class NumberSprite : public Sprite
{
public:
    NumberSprite();
    NumberSprite(
        std::filesystem::path               path,
        std::vector<std::vector<glm::vec2>> texCoords);
    NumberSprite(
        std::shared_ptr<Graphics::Texture2D> texture,
        std::vector<std::vector<glm::vec2>>  texCoords);

    NumericPosition NumberPosition = NumericPosition::MID;
    bool            FillWithZeros = false;
    bool            AlphaBlend = false;
    int             MaxDigits = 0;
    int             Offset = 0;

    void Draw(int value);
    void CalculateSize() override;

protected:
    void OnDraw() override;
    void AddToQueue();
    int  NumberToDraw = 0;
};