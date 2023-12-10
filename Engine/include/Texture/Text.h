#pragma once
#include "Color3.h"
#include "Rendering/WindowsTypes.h"
#include "UDim2.h"
#include "Vector2.h"
#include <filesystem>

struct ImVec2;

class Text
{
public:
    Text();
    Text(int size);
    ~Text();

    // void Draw(std::string);
    void Draw(std::wstring, ...);
    void Draw(std::u8string, ...);
    void Draw(std::string, ...);
    int  CalculateSize(std::u8string u);

    bool DrawOverEverything;
    Rect Clip;

    float             Transparency;
    float             Rotation;
    int               Size;
    UDim2             Position;
    typename ::Color3 Color3;
    Vector2           AnchorPoint;
    Vector2           AbsolutePosition;
    Vector2           AbsoluteSize;

private:
    void InternalDraw(std::u8string &text);

    int    rotation_start_index;
    void   ImRotationStart();
    ImVec2 ImRotationCenter();
    void   ImRotationEnd(float rad, ImVec2 center);
};