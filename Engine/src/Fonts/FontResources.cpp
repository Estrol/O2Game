#pragma warning(disable: 4838) // Goddamit
#pragma warning(disable: 4309)

#include <iostream>
#include <filesystem>
#include <mutex>

#include "Configuration.h"
#include "Rendering/Window.h"
#include "Fonts/FontResources.h"

#include "Imgui/imgui.h"
#include "../Data/Imgui/misc/freetype/imgui_freetype.h"
#include "../Data/Imgui/imgui_impl_sdl2.h"
#include "../Data/Imgui/imgui_impl_vulkan.h"
#include "../Data/Imgui/imgui_impl_sdlrenderer2.h"

#include "Rendering/Renderer.h"
#include "Rendering/Vulkan/VulkanEngine.h"

// BEGIN FONT FALLBACK

#include "FallbackFonts/arial.ttf.h"
#include "FallbackFonts/jp.ttf.h"

// END FONT FALLBACK

static ImFont* gImFontButton = nullptr;
static ImFont* gImFontSlider = nullptr;
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

void FontResources::PreloadFontCaches() {
    std::cout << "[Font] Preload font on progress!" << std::endl;
    std::lock_guard<std::mutex> lock(mutex);

    if (!gImFontRebuild) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->Clear();
    io.Fonts->ClearFonts();

    Window* wnd = Window::GetInstance();

    float originScale = (wnd->GetBufferWidth() + wnd->GetBufferHeight()) / 15.6f;
    float targetScale = (wnd->GetWidth() + wnd->GetHeight()) / 15.6f;

    float fontSize = std::round(13.0f * (targetScale / originScale));

    auto SkinName = Configuration::Load("Game", "Skin");
    auto skinPath = Configuration::Skin_GetPath(SkinName);
    auto fontPath = skinPath / "Fonts";

    auto font = fontPath / "normal.ttf";
    auto jpFont = fontPath / "jp.ttf";

    ImFontConfig conf;
    conf.OversampleH = conf.OversampleV = 0;
    conf.PixelSnapH = true;
    conf.SizePixels = fontSize;
    conf.GlyphOffset.y = 1.0f * (fontSize / 16.0f);
    conf.FontDataOwnedByAtlas = false;

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

    float iBtnFontSz = std::round(20.0f * (targetScale / originScale));
    conf.MergeMode = false;
    conf.SizePixels = iBtnFontSz;
    conf.GlyphOffset.y = 1.0f * (iBtnFontSz / 16.0f);
    if (std::filesystem::exists(font)) {
        gImFontButton = io.Fonts->AddFontFromFileTTF((const char*)font.u8string().c_str(), iBtnFontSz, &conf);
        gImFontSlider = io.Fonts->AddFontFromFileTTF((const char*)font.u8string().c_str(), iBtnFontSz, &conf);
    }
    else {
        gImFontButton = io.Fonts->AddFontFromMemoryTTF((void*)arial_ttf, sizeof(arial_ttf), iBtnFontSz, &conf);
        gImFontSlider = io.Fonts->AddFontFromMemoryTTF((void*)arial_ttf, sizeof(arial_ttf), iBtnFontSz, &conf);
    }

    if (Renderer::GetInstance()->IsVulkan()) {
        auto vulkan = Renderer::GetInstance()->GetVulkanEngine();

        //execute a gpu command to upload imgui font textures
        vulkan->immediate_submit([&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

        //clear font textures from cpu data
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
    else {
        ImGui_ImplSDLRenderer2_DestroyDeviceObjects();
        ImGui_ImplSDLRenderer2_DestroyFontsTexture();
    }

    gImFontRebuild = false;

    std::cout << "[Font] Preload font completed!" << std::endl;
}

void FontResources::Rebuild() {
    std::lock_guard<std::mutex> lock(mutex);

    if (!gImFontRebuild) {
        return;
    }

    ImFontAtlas* atlas = ImGui::GetIO().Fonts;
    atlas->Clear();

    atlas->Build();

    if (Renderer::GetInstance()->IsVulkan()) {
        auto vulkan = Renderer::GetInstance()->GetVulkanEngine();

        //execute a gpu command to upload imgui font textures
        vulkan->immediate_submit([&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

        //clear font textures from cpu data
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    gImFontRebuild = false;
}

bool FontResources::ShouldRebuild() {
    return gImFontRebuild;
}

void FontResources::DoRebuild() {
    gImFontRebuild = true;
}
ImFont* FontResources::GetButtonFont() {
    return gImFontButton;
}
ImFont* FontResources::GetReallyBigFontForSlider() {
    return gImFontSlider;
}