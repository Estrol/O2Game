#pragma once
#include <filesystem>
#include "UDim2.h"
#include "Color3.h"
#include "Vector2.h"

struct ImVec2;

class Text {
public:
    Text();
    Text(int size);
    ~Text();

    // void Draw(std::string);
    void Draw(std::wstring, ...);
    void Draw(std::u8string, ...);
    void Draw(std::string, ...);
    int CalculateSize(std::u8string u);

    bool DrawOverEverything;

    float Transparency;
    float Rotation;
    int Size;
    UDim2 Position;
    Color3 Color3;
    Vector2 AnchorPoint;
    Vector2 AbsolutePosition;
    Vector2 AbsoluteSize;
private:
    int rotation_start_index;
    void ImRotationStart();
    ImVec2 ImRotationCenter();
    void ImRotationEnd(float rad, ImVec2 center);
};