#include "ImguiUtil.hpp"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

namespace {
	bool HasAQueueFrame = false;
}

void ImguiUtil::NewFrame() {
	if (!HasAQueueFrame) {
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
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

void ImguiUtil::Reset() {
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplSDL2_ResetFrame();

	HasAQueueFrame = false;
}
