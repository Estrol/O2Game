#pragma once
#include <Rendering/WindowsTypes.h>
#include <Imgui/imgui.h>
#include <Texture/Vector2.h>
#include <Texture/UDim2.h>
#include <Texture/Color3.h>
#include <vector>

class UIBase {
public:
    UIBase();
    virtual ~UIBase();

    Vector2 AbsolutePosition;
    Vector2 AbsoluteSize;

    UDim2 Position;
    UDim2 Size;

    UIBase* Parent;
    bool ClampToParent;

    Color3 Color3;
    Vector2 AnchorPoint;

    float Transparency;
    float Rotation;

    virtual void Draw();
    virtual void Draw(Rect clipRect);

    void CalculateSize();

protected:
    virtual void OnDraw();

    void RotatePoint(float x, float y, float cx, float cy, float theta, float& out_x, float& out_y);

    Rect clipRect = {};
    std::vector<ImDrawVert> m_vertices;
    std::vector<uint16_t> m_indices;

private:
    void DrawVertices();
};