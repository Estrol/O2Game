#include "FontResources.hpp"

/* Just include ^_^ */
#include "FallbackFonts/arial.ttf.h"
#include "Imgui/imgui.h"

static std::map<std::string, void*> gFontCaches;
static std::map<std::string, ImFont*> gImFontCaches;

namespace FontResources {
	void PreloadFontCaches() {
		gFontCaches["Arial"] = (void*)arial_ttf;

		// Force load default;
		FontResources::Load("Arial", 16);

		// Precache it from font size 5 to 36
		for (int i = 5; i <= 36; i++) {
			for (auto& it : gFontCaches) {
				Load(it.first, i);
			}
		}
	}

	ImFont* Load(std::string name, int size) {
		if (gFontCaches.find(name) == gFontCaches.end()) {
			throw std::exception("INVALID_FONT_NAME");
		}

		void* binary = gFontCaches[name];
		name += std::to_string(size);

		if (gImFontCaches.find(name) != gImFontCaches.end()) {
			return gImFontCaches[name];
		}

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImFont* font = io.Fonts->AddFontFromMemoryTTF(binary, size, size, NULL, io.Fonts->GetGlyphRangesJapanese());

		gImFontCaches[name] = font;
		return font;
	}
}