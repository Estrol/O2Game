#include "Console.h"
#include <Imgui/ImguiUtil.h>
#include <Imgui/imgui.h>
#include <Texture/MathUtils.h>

namespace {
    std::string outputBuffer = {};
    bool        showConsole = false;
} // namespace

void Console::Send(std::string output)
{
    outputBuffer += output + "\n";
}

void Console::Draw()
{
    ImguiUtil::NewFrame();

    if (ImGui::IsKeyPressed(ImGuiKey_F11, false)) {
        showConsole = !showConsole;
    }

    ImGui::SetNextWindowSize(MathUtil::ScaleVec2(400, 400), ImGuiCond_FirstUseEver);
    if (showConsole && ImGui::Begin("Console", &showConsole, 0)) {
        if (ImGui::Button("Clear Console")) {
            outputBuffer.clear();
        }

        auto size = ImGui::GetWindowSize();
        size -= MathUtil::ScaleVec2(10, 50);

        if (ImGui::BeginChild("#console_child_window", size, true, 0)) {
            ImGui::Text("%s", outputBuffer.c_str());

            ImGui::EndChild();
        }

        ImGui::End();
    }
}