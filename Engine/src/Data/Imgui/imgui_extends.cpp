#include <Imgui/imgui_extends.h>
#include <Imgui/imgui_internal.h>
#include <stdio.h>
#include <stdarg.h>

void imgui_extends::TextAligment(const char* text, float alignment)  {
    ImGuiStyle& style = ImGui::GetStyle();

    float size = ImGui::CalcTextSize(text).x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;

    float off = (avail - size) * alignment;
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    return ImGui::Text("%s", text);
}
void imgui_extends::TextBackground(ImVec4 bgColor, ImVec2 size, const char *format, ...) {
    char buf[1024];
    {
        va_list args;
        va_start(args, format);
        vsnprintf(buf, IM_ARRAYSIZE(buf), format, args);
        va_end(args);
    }

    {
        char tmpBuf[1024];
        const char* tmpFormat = " %s ";
        snprintf(tmpBuf, IM_ARRAYSIZE(tmpBuf), tmpFormat, buf);

        strcpy(buf, tmpBuf);
    }

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);

    ImVec2 text_size = ImGui::CalcTextSize(buf, NULL, true);
    if (size.x == 0) {
        size.x = text_size.x;
    }

    if (size.y == 0) {
        size.y = text_size.y;
    }

    // add 2 px padding
    size.y += 5;
    size.x += 4;

    ImGui::GetWindowDrawList()->AddRectFilled(text_pos, ImVec2(text_pos.x + size.x, text_pos.y + size.y), ImGui::GetColorU32(bgColor));
    return ImGui::Text("%s", buf);
}