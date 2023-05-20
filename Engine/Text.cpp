#include "Text.hpp"
#include "Renderer.hpp"
#include "Imgui/imgui_internal.h"
#include "Imgui/ImguiUtil.hpp"
#include "FontResources.hpp"
#include <codecvt>

Text::Text() {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    m_font_ptr = g.Font;

    Rotation = 0.0f;
    Transparency = 0.0f;
    Size = m_font_ptr ? m_font_ptr->FontSize : 13;
    Color3 = { 1.0, 1.0, 1.0 };
    rotation_start_index = 0;
    m_current_font_size = Size;
    DrawOverEverything = false;
}

Text::Text(std::string fontName, int sz) : Text() {
    m_current_font_name = fontName;
    m_current_font_size = sz;
    Size = sz;

    ImFont* font = FontResources::Load(fontName, Size);
    if (font) {
        m_font_ptr = font;
    }
}

void Text::SetFont(ImFont* fontPtr) {
    m_font_ptr = fontPtr;
}

void Text::SetFont(std::string fontName) {
    m_current_font_name = fontName;
    m_font_ptr = FontResources::Load(fontName, Size);
}

void Text::Draw(std::string text) {
    Draw(std::u8string(text.begin(), text.end()));
}

void Text::Draw(std::wstring text) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> mcov;
	Draw(mcov.to_bytes(text));
}

void Text::Draw(std::u8string text) {
    Size = std::clamp(Size, 5, 36);

    if (m_current_font_size != Size) {
        m_current_font_size = Size;
        m_font_ptr = FontResources::Load(m_current_font_name, Size);
    }

    float radians = ((Rotation + 90.0f) * ((22.0f / 7.0f) / 180.0f));
    float red = Color3.R;
    float green = Color3.G;
    float blue = Color3.B;
    float alpha = Transparency;

    auto& displaySize = ImGui::GetIO().DisplaySize;

    ImguiUtil::BeginText();

    ImDrawList* draw_list;
    if (DrawOverEverything) {
        draw_list = ImGui::GetForegroundDrawList();
    }
    else {
        draw_list = ImGui::GetWindowDrawList();
    }
	
    auto textSize = ImGui::CalcTextSizeWithSize((const char*)text.c_str(), Size);

    Window* window = Window::GetInstance();
    int wWidth = window->GetWidth();
    int wHeight = window->GetHeight();

    LONG xPos = static_cast<LONG>(wWidth * Position.X.Scale) + static_cast<LONG>(Position.X.Offset);
    LONG yPos = static_cast<LONG>(wHeight * Position.Y.Scale) + static_cast<LONG>(Position.Y.Offset);
	
    ImRotationStart();

    draw_list->AddText(
        m_font_ptr, 
        Size, 
        ImVec2(xPos, yPos), 
        ImColor(255 * red, 255 * green, 255 * blue), 
        (const char*)text.c_str()
    );

    ImRotationEnd(radians, ImRotationCenter());
    ImguiUtil::EndText();
}


Text::~Text() {

}

void Text::ImRotationStart() {
    rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
}

ImVec2 Text::ImRotationCenter() {
    ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

    const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

    return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2); // or use _ClipRectStack?
}

void Text::ImRotationEnd(float rad, ImVec2 center) {
    float s = sin(rad), c = cos(rad);
    center = ImRotate(center, s, c) - center;

    auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
}