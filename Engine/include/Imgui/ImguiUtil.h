#pragma once
struct ImVec2;

namespace ImguiUtil {
	void NewFrame();
	void BeginText(ImVec2 pos, ImVec2 size);
	void EndText();
	bool HasFrameQueue();
	void Reset();
	void SetVulkan(bool v);
}