/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Math/Color3.h>
#include <algorithm>
#include <cmath>
#include <iostream>

Color3::Color3(float r, float g, float b)
{
    R = std::clamp(r, 0.0f, 1.0f);
    G = std::clamp(g, 0.0f, 1.0f);
    B = std::clamp(b, 0.0f, 1.0f);
}

Color3 Color3::fromRGB(float r, float g, float b)
{
    return { r / 255.0f, g / 255.0f, b / 255.0f };
}

Color3 Color3::fromHSV(int hue, int saturnation, int value)
{
    float R, G, B;

    float h = hue / 60.0f;
    float s = saturnation / 100.0f;
    float v = value / 100.0f;
    float c = v * s;
    float x = c * (1.0f - std::abs(fmod(h, 2.0f) - 1.0f));
    float m = v - c;

    if (h >= 0 && h < 1) {
        R = c;
        G = x;
        B = 0;
    } else if (h >= 1 && h < 2) {
        R = x;
        G = c;
        B = 0;
    } else if (h >= 2 && h < 3) {
        R = 0;
        G = c;
        B = x;
    } else if (h >= 3 && h < 4) {
        R = 0;
        G = x;
        B = c;
    } else if (h >= 4 && h < 5) {
        R = x;
        G = 0;
        B = c;
    } else if (h >= 5 && h < 6) {
        R = c;
        G = 0;
        B = x;
    } else {
        R = 0;
        G = 0;
        B = 0;
    }

    return { R, G, B };
}

Color3 Color3::fromHex(std::string hexValue)
{
    if (hexValue[0] == '#') {
        hexValue = hexValue.substr(1);
    }

    if (hexValue.size() == 8) {
        hexValue = hexValue.substr(2);
    }

    if (hexValue.size() != 6) {
        return { 0, 0, 0 };
    }

    int r = std::stoi(hexValue.substr(0, 2), nullptr, 16);
    int g = std::stoi(hexValue.substr(2, 2), nullptr, 16);
    int b = std::stoi(hexValue.substr(4, 2), nullptr, 16);

    return { r / 255.0f, g / 255.0f, b / 255.0f };
}

Color3 Color3::Lerp(Color3 dest, float alpha)
{
    return (*this) * (1.0f - alpha) + (dest * alpha);
}

int Color3::ToHSV()
{
    std::cout << "Color3::ToHSV not implemented yet and return 0" << std::endl;
    return 0;
}

std::string Color3::ToHex()
{
    std::string result = "#";
    result += std::to_string(static_cast<int>(R * 255));
    result += std::to_string(static_cast<int>(G * 255));
    result += std::to_string(static_cast<int>(B * 255));

    return result;
}

// operator
Color3 Color3::operator+(Color3 const &color)
{
    return { this->R + color.R, this->G + color.G, this->B + color.B };
}

Color3 Color3::operator-(Color3 const &color)
{
    return { this->R - color.R, this->G - color.G, this->B - color.B };
}

Color3 Color3::operator*(Color3 const &color)
{
    return { this->R * color.R, this->G * color.G, this->B * color.B };
}

Color3 Color3::operator/(Color3 const &color)
{
    return { this->R / color.R, this->G / color.G, this->B / color.B };
}

Color3 Color3::operator*(float const &value)
{
    return { this->R * value, this->G * value, this->B * value };
}

Color3 Color3::operator/(float const &value)
{
    return { this->R / value, this->G / value, this->B / value };
}

bool Color3::operator==(Color3 const &color)
{
    return this->R == color.R && this->G == color.G && this->B == color.B;
}

bool Color3::operator!=(Color3 const &color)
{
    return this->R != color.R || this->G != color.G || this->B != color.B;
}

bool Color3::operator<=(Color3 const &color)
{
    return this->R <= color.R && this->G <= color.G && this->B <= color.B;
}

bool Color3::operator>=(Color3 const &color)
{
    return this->R >= color.R && this->G >= color.G && this->B >= color.B;
}

bool Color3::operator<(Color3 const &color)
{
    return this->R < color.R && this->G < color.G && this->B < color.B;
}

bool Color3::operator>(Color3 const &color)
{
    return this->R > color.R && this->G > color.G && this->B > color.B;
}