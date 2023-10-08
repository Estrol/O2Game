#include "Imgui/ImguiUtil.h"
#include "Imgui/imgui.h"
#include "Imgui/imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdlrenderer2.h"

// Duhh very BAD code!!!
#include "Rendering/Vulkan/VulkanEngine.h"

namespace {
	bool HasAQueueFrame = false;
	bool vulkan = false;
}

void ImguiUtil::NewFrame() {
	if (!HasAQueueFrame) {
		if (vulkan) {
			VulkanEngine::GetInstance()->imgui_begin();
		}
		else {
			ImGui_ImplSDLRenderer2_NewFrame();
			ImGui_ImplSDL2_NewFrame();
		}

		ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg] = ImColor(24, 24, 24, static_cast<int>(255.0 * 0.5));

		ImGui::NewFrame();

		HasAQueueFrame = true;
	}
}

void ImguiUtil::BeginText(ImVec2 pos, ImVec2 size) {
	if (!ImGui_ImplSDL2_HasAFrame()) {
		NewFrame();
	}

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::Begin(
		"##Main",
		nullptr,
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoBringToFrontOnFocus
	);
}

void ImguiUtil::EndText() {
	ImGui::End();
}

bool ImguiUtil::HasFrameQueue() {
	return HasAQueueFrame;
}

void ImguiUtil::SetVulkan(bool v) {
	vulkan = v;
}

void ImguiUtil::Reset() {
	ImGui::EndFrame();

	if (vulkan) {
		VulkanEngine::GetInstance()->imgui_end();
	}
	else {
		ImGui::Render();
		ImGui_ImplSDL2_ResetFrame();
	}

	HasAQueueFrame = false;
}

void ImguiUtil::Text::ColoredBackground(ImVec4 bgColor, ImVec2 size, const char* format, ...) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window == nullptr) {
		return;
	}

    ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);

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
    ImGui::Text("%s", buf);
}