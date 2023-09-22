#pragma once
struct ImVec2;
struct ImVec4;

namespace ImguiUtil {
	void NewFrame();
	void BeginText(ImVec2 pos, ImVec2 size);
	void EndText();
	bool HasFrameQueue();
	void Reset();
	void SetVulkan(bool v);

	namespace Text {
		void ColoredBackground(ImVec4 bgColor, ImVec2 size, const char* format, ...);
	}
}