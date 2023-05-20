#pragma once
#include <filesystem>
#include "Imgui/imgui_internal.h"
#include "UDim2.hpp"
#include "Color3.hpp"
#include "Vector2.hpp"

class Text {
public:
    Text();
    Text(std::string fontName, int size = 13);
    ~Text();

    void SetFont(ImFont* fontPtr);
    void SetFont(std::string fontName);
    void Draw(std::wstring);
    void Draw(std::u8string);
    void Draw(std::string);

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
    int m_current_font_size;
    ImFont* m_font_ptr;
    std::string m_current_font_name;

    int rotation_start_index;
    void ImRotationStart();
    ImVec2 ImRotationCenter();
    void ImRotationEnd(float rad, ImVec2 center);
};