#include "ImguiUtil.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdlrenderer2.h"

// Duhh very BAD code!!!
#include "../VulkanDriver/VulkanEngine.h"

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

		ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg] = ImColor(24, 24, 24, 255 * 0.5);

		ImGui::NewFrame();

		HasAQueueFrame = true;
	}
}

void ImguiUtil::BeginText() {
	if (!ImGui_ImplSDL2_HasAFrame()) {
		NewFrame();
	}

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin(
		"##Main",
		nullptr,
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoBackground
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
