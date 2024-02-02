/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __COLOR3_H_
#define __COLOR3_H_
#include <string>

class Color3
{
public:
    Color3() = default;
    Color3(float r, float g, float b);
    static Color3 fromRGB(float r, float g, float b);
    static Color3 fromHSV(int hue, int saturnation, int value);
    static Color3 fromHex(std::string hexValue);

    Color3      Lerp(Color3 dest, float alpha);
    int         ToHSV();
    std::string ToHex();

    float R, G, B;

    // operator
    Color3 operator+(Color3 const &color);
    Color3 operator-(Color3 const &color);
    Color3 operator*(Color3 const &color);
    Color3 operator/(Color3 const &color);
    Color3 operator*(float const &value);
    Color3 operator/(float const &value);
    bool   operator==(Color3 const &color);
    bool   operator!=(Color3 const &color);
    bool   operator<=(Color3 const &color);
    bool   operator>=(Color3 const &color);
    bool   operator<(Color3 const &color);
    bool   operator>(Color3 const &color);
};

#endif