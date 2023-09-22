#pragma once
#include "imgui.h"

namespace imgui_extends {
    void TextAligment(const char* text, float alignment = 0.0f);
    void TextBackground(ImVec4 bgColor, ImVec2 size, const char* format, ...);
}