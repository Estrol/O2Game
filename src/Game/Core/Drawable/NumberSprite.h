/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "Image.h"
#include <map>
#include <string>
#include <vector>

enum class NumericPosition {
    LEFT,
    MID,
    RIGHT
};

NumericPosition IntToPos(int i);

class NumberSprite
{
public:
    NumberSprite() = default;
    NumberSprite(std::vector<std::filesystem::path> numericsPath);
    ~NumberSprite();

    UDim2           Position;
    UDim2           Position2;
    Color3          Color;
    UDim2           Size;
    Vector2         AnchorPoint;
    NumericPosition NumberPosition = NumericPosition::MID;
    bool            FillWithZeros = false;
    bool            AlphaBlend = false;
    int             MaxDigits = 0;
    int             Offset = 0;

    void Draw(int value);

protected:
    std::vector<std::shared_ptr<Image>> m_numericsTexture;
    std::map<int, Rect>                 m_numbericsWidth;
};