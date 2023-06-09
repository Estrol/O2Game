#include "FontResources.hpp"
#include "../Game/Resources/Configuration.hpp"
#include "Window.hpp"
#include <mutex>

#include "Imgui/imgui.h"
#include "Imgui/misc/freetype/imgui_freetype.h"

/* Just include ^_^ */
#pragma warning(disable: 4838) // Goddamit
#pragma warning(disable: 4309)

#include "FallbackFonts/arial.ttf.h"
#include "FallbackFonts/jp.ttf.h"
#include "Imgui/imgui_impl_sdlrenderer2.h"

static std::map<std::string, void*> gFontCaches;
static std::map<std::string, ImFont*> gImFontCaches;
static bool gImFontRebuild = true;
static std::mutex mutex;

double calculateDisplayDPI(double diagonalDPI, double horizontalDPI, double verticalDPI) {
	double horizontalSquared = pow(horizontalDPI, 2);
	double verticalSquared = pow(verticalDPI, 2);
	double sumSquared = horizontalSquared + verticalSquared;
	double root = sqrt(sumSquared);
	double displayDPI = root / diagonalDPI;

	return displayDPI;
}

namespace FontResources {
	void PreloadFontCaches() {
		std::lock_guard<std::mutex> lock(mutex);

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->Clear();
		io.Fonts->ClearFonts();

		Window* wnd = Window::GetInstance();

		float originScale = (wnd->GetBufferWidth() + wnd->GetBufferHeight()) / 15.6;
		float targetScale = (wnd->GetWidth() + wnd->GetHeight()) / 15.6;

		int fontSize = 13 * (targetScale / originScale);

		auto SkinName = Configuration::Load("Game", "Skin");
		auto skinPath = Configuration::Skin_GetPath(SkinName);
		auto fontPath = skinPath / "Fonts";

		auto font = fontPath / "normal.ttf";
		auto jpFont = fontPath / "jp.ttf";

		ImFontConfig conf;
		conf.OversampleH = conf.OversampleH = 3;
		conf.PixelSnapH = true;
		conf.SizePixels = fontSize;
		conf.GlyphOffset.y = 1 * (fontSize / 16.0);

		if (std::filesystem::exists(font)) {
			io.Fonts->AddFontFromFileTTF((const char*)font.u8string().c_str(), fontSize, &conf);
		}
		else {
			io.Fonts->AddFontFromMemoryTTF((void*)arial_ttf, sizeof(arial_ttf), fontSize, &conf);
		}

		conf.MergeMode = true;
		io.Fonts->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
		io.Fonts->FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_NoHinting;

		if (std::filesystem::exists(jpFont)) {
			io.Fonts->AddFontFromFileTTF((const char*)jpFont.u8string().c_str(), fontSize, &conf, io.Fonts->GetGlyphRangesJapanese());
		}
		else {
			io.Fonts->AddFontFromMemoryTTF((void*)jp_ttf, sizeof(jp_ttf), fontSize, &conf, io.Fonts->GetGlyphRangesJapanese());
		}

		io.Fonts->Build();
		gImFontRebuild = false;

		ImGui_ImplSDLRenderer2_DestroyDeviceObjects();
		ImGui_ImplSDLRenderer2_DestroyFontsTexture();
	}

	void Rebuild() {
		std::lock_guard<std::mutex> lock(mutex);

		if (!gImFontRebuild) {
			return;
		}

		ImFontAtlas* atlas = ImGui::GetIO().Fonts;
		atlas->Clear();
		
		

		atlas->Build();

		gImFontRebuild = false;
	}

	bool ShouldRebuild() {
		return gImFontRebuild;
	}

	void DoRebuild() {
		gImFontRebuild = true;
	}
}