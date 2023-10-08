#pragma once
#include <vector>
#include <Imgui/imgui.h>

namespace UIMath {
    std::vector<ImDrawVert> GenerateQuadVertices(ImVec4 rect, ImVec4 color, float radius);
}