#include "Texture/Text.h"
#include "Rendering/Renderer.h"
#include "Imgui/imgui_internal.h"
#include "Imgui/ImguiUtil.h"
#include "Fonts/FontResources.h"
#include <codecvt>
#include "Texture/MathUtils.h"
#include <cmath>
#include <algorithm>

#if _WIN32
#include <windows.h>
#elif
#include <locale>
#endif

constexpr auto MAX_FORMATTED_TEXT_SIZE = 2048;

Text::Text() {
    Rotation = 0.0f;
    Transparency = 0.0f;
    Size = 13;
    Color3 = { 1.0, 1.0, 1.0 };
    rotation_start_index = 0;
    DrawOverEverything = false;
}

Text::Text(int sz) : Text() {
    Size = sz;
}

void Text::Draw(std::wstring text, ...) {
#if _WIN32
    int len = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, NULL, 0, NULL, NULL);
    char* buf = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, buf, len, NULL, NULL);
    std::u8string u8str(buf, buf + len);

    delete[] buf;
#else
    // use linux native without std
    int len = wctomb(NULL, text.c_str());
    char* buf = new char[len];
    wctomb(buf, text.c_str());
    std::u8string u8str(buf, buf + len);

    delete[] buf;
#endif

    va_list args;
    va_start(args, text);
    char buffer[MAX_FORMATTED_TEXT_SIZE];
    int b_sz = vsnprintf(buffer, MAX_FORMATTED_TEXT_SIZE, (const char*)u8str.c_str(), args);
    std::u8string formated(buffer, buffer + b_sz);
    va_end(args);

    InternalDraw(formated);
}

void Text::Draw(std::u8string text, ...) {
    va_list args;
    va_start(args, text);
    char buffer[MAX_FORMATTED_TEXT_SIZE];
    int b_sz = vsnprintf(buffer, MAX_FORMATTED_TEXT_SIZE, (const char*)text.c_str(), args);
    std::u8string formated(buffer, buffer + b_sz);
    va_end(args);

    InternalDraw(formated);
}

void Text::Draw(std::string text, ...) {
    va_list args;
    va_start(args, text);
    char buffer[MAX_FORMATTED_TEXT_SIZE];
    int b_sz = vsnprintf(buffer, MAX_FORMATTED_TEXT_SIZE, text.c_str(), args);
    std::u8string formated(buffer, buffer + b_sz);
    va_end(args);

    InternalDraw(formated);
}

void Text::InternalDraw(std::u8string& formated) {
    Size = std::clamp(Size, 5, 36);

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

    Window* wnd = Window::GetInstance();
    float originScale = (wnd->GetBufferWidth() + wnd->GetBufferHeight()) / 15.6f;
	float targetScale = (wnd->GetWidth() + wnd->GetHeight()) / 15.6f;

	float scale = targetScale / originScale;
	
    auto textSize = ImGui::CalcTextSizeWithSize((const char*)formated.c_str(), Size * scale);

    Window* window = Window::GetInstance();
    int wWidth = window->GetWidth();
    int wHeight = window->GetHeight();

    float xPos = static_cast<float>((wWidth * Position.X.Scale) + Position.X.Offset);
    float yPos = static_cast<float>((wHeight * Position.Y.Scale) + Position.Y.Offset);

    xPos *= wnd->GetWidthScale();
	yPos *= wnd->GetHeightScale();
	
    ImRotationStart();

    draw_list->AddText(
        NULL, 
        Size * scale,
        ImVec2(xPos, yPos) - MathUtil::ScaleVec2(0, 2.5),
        ImColor(255 * red, 255 * green, 255 * blue), 
        (const char*)formated.c_str()
    );

    ImRotationEnd(radians, ImRotationCenter());
    ImguiUtil::EndText();
}


Text::~Text() {

}

int Text::CalculateSize(std::u8string text) {
    ImguiUtil::NewFrame();

    Window* wnd = Window::GetInstance();
    float originScale = (wnd->GetBufferWidth() + wnd->GetBufferHeight()) / 15.6f;
    float targetScale = (wnd->GetWidth() + wnd->GetHeight()) / 15.6f;

    float scale = targetScale / originScale;

    return static_cast<int>(ImGui::CalcTextSizeWithSize((const char*)text.c_str(), Size * scale).x);
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