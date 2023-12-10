#pragma warning(disable : 4838) // Goddamit
#pragma warning(disable : 4309)

#include <Misc/md5.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>

#include "Configuration.h"
#include "Fonts/FontResources.h"
#include "Rendering/Window.h"

#include "../Data/Imgui/imgui_impl_sdl2.h"
#include "../Data/Imgui/imgui_impl_sdlrenderer2.h"
#include "../Data/Imgui/imgui_impl_vulkan.h"
#include "../Data/Imgui/misc/freetype/imgui_freetype.h"
#include "Imgui/imgui.h"

#include "Rendering/Renderer.h"
#include "Rendering/Vulkan/VulkanEngine.h"

// BEGIN FONT FALLBACK

#include "FallbackFonts/arial.ttf.h"
#include "FallbackFonts/ch.ttf.h"
#include "FallbackFonts/jp.ttf.h"
#include "FallbackFonts/kr.ttf.h"
#include <Logs.h>

// END FONT FALLBACK

namespace {
    int        CurrentFontIndex = 0;
    bool       gImFontRebuild = true;
    std::mutex mutex;

    FontResolution             Font;
    std::map<TextRegion, bool> gRegions = {
        { TextRegion::Chinese, false },
        { TextRegion::Japanese, false },
        { TextRegion::Korean, false },
    };
} // namespace

double calculateDisplayDPI(double diagonalDPI, double horizontalDPI, double verticalDPI)
{
    double horizontalSquared = pow(horizontalDPI, 2);
    double verticalSquared = pow(verticalDPI, 2);
    double sumSquared = horizontalSquared + verticalSquared;
    double root = sqrt(sumSquared);
    double displayDPI = root / diagonalDPI;

    return displayDPI;
}

void FontResources::RegisterFontIndex(int idx, int width, int height)
{
    FontResolution font;
    font.Index = idx;
    font.Width = width;
    font.Height = height;

    Font = font;
}

void FontResources::ClearFontIndex()
{
    // Fonts.clear();
}

void FontResources::PreloadFontCaches()
{
    Logs::Puts("[FontResources] Preparing font to load");
    std::lock_guard<std::mutex> lock(mutex);

    if (!gImFontRebuild) {
        return;
    }

    if (Font.Height == 0 || Font.Width == 0) {
        Font.Width = 800, Font.Height = 600;
        Font.Index = 0;
    }

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.Fonts->Clear();
    io.Fonts->ClearFonts();

    GameWindow *wnd = GameWindow::GetInstance();

    auto skinPath = Configuration::Font_GetPath();
    auto fontPath = skinPath / "Fonts";

    auto normalfont = fontPath / "normal.ttf";
    auto jpFont = fontPath / "jp.ttf";
    auto krFont = fontPath / "kr.ttf";
    auto chFont = fontPath / "ch.ttf";

    {
        float originScale = (wnd->GetBufferWidth() + wnd->GetBufferHeight()) / 15.6f;
        float targetScale = (wnd->GetWidth() + wnd->GetHeight()) / 15.6f;

        float fontSize = ::round(13.0f * (targetScale / originScale));

        ImFontConfig conf;
        conf.OversampleH = conf.OversampleV = 0;
        conf.PixelSnapH = true;
        conf.SizePixels = fontSize;
        conf.GlyphOffset.y = 1.0f * (fontSize / 16.0f);
        conf.FontDataOwnedByAtlas = false;

        io.Fonts->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
        io.Fonts->FontBuilderFlags = ImGuiFreeTypeBuilderFlags_NoHinting;

        {
            if (std::filesystem::exists(normalfont)) {
                Font.Font = io.Fonts->AddFontFromFileTTF((const char *)normalfont.u8string().c_str(), fontSize, &conf);
            } else {
                Font.Font = io.Fonts->AddFontFromMemoryTTF((void *)get_arial_font_data(), get_arial_font_size(), fontSize, &conf);
            }
        }

        {
            conf.MergeMode = true;

            for (auto &[region, enabled] : gRegions) {
                if (enabled) {
                    switch (region) {
                        case TextRegion::Japanese:
                        {
                            if (std::filesystem::exists(jpFont)) {
                                io.Fonts->AddFontFromFileTTF((const char *)jpFont.u8string().c_str(), fontSize, &conf, io.Fonts->GetGlyphRangesJapanese());
                            }

                            break;
                        }

                        case TextRegion::Korean:
                        {
                            if (std::filesystem::exists(krFont)) {
                                io.Fonts->AddFontFromFileTTF((const char *)krFont.u8string().c_str(), fontSize, &conf, io.Fonts->GetGlyphRangesKorean());
                            }

                            break;
                        }

                        case TextRegion::Chinese:
                        {
                            if (std::filesystem::exists(chFont)) {
                                io.Fonts->AddFontFromFileTTF((const char *)chFont.u8string().c_str(), fontSize, &conf, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
                            }

                            break;
                        }
                    }
                }
            }

            bool result = io.Fonts->Build();
            if (!result) {
                throw std::runtime_error("Failed to build font atlas");
            }
        }

        {
            conf.MergeMode = false;

            io.Fonts->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
            io.Fonts->FontBuilderFlags = ImGuiFreeTypeBuilderFlags_NoHinting;

            float iBtnFontSz = ::round(20.0f * (targetScale / originScale));
            conf.SizePixels = iBtnFontSz;
            conf.GlyphOffset.y = 1.0f * (iBtnFontSz / 16.0f);

            if (std::filesystem::exists(normalfont)) {
                Font.ButtonFont = io.Fonts->AddFontFromFileTTF((const char *)normalfont.u8string().c_str(), iBtnFontSz, &conf);
                Font.SliderFont = io.Fonts->AddFontFromFileTTF((const char *)normalfont.u8string().c_str(), iBtnFontSz, &conf);
            } else {
                Font.ButtonFont = io.Fonts->AddFontFromMemoryTTF((void *)get_arial_font_data(), get_arial_font_size(), iBtnFontSz, &conf);
                Font.SliderFont = io.Fonts->AddFontFromMemoryTTF((void *)get_arial_font_data(), get_arial_font_size(), iBtnFontSz, &conf);
            }

            io.Fonts->Build();
        }
    }

    if (Renderer::GetInstance()->IsVulkan()) {
        auto vulkan = Renderer::GetInstance()->GetVulkanEngine();

        // execute a gpu command to upload imgui font textures
        vulkan->immediate_submit([&](VkCommandBuffer cmd) {
            bool success = ImGui_ImplVulkan_CreateFontsTexture(cmd);
            if (!success) {
                throw std::runtime_error("[Vulkan] Failed to create fonts texture!");
            }
        });

        // clear font textures from cpu data
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    } else {
        ImGui_ImplSDLRenderer2_DestroyDeviceObjects();
        ImGui_ImplSDLRenderer2_DestroyFontsTexture();
    }

    gImFontRebuild = false;

    Logs::Puts("[FontResources] Preload font completed!");
}

bool FontResources::ShouldRebuild()
{
    return gImFontRebuild;
}

void FontResources::DoRebuild()
{
    gImFontRebuild = true;
}

void FontResources::LoadFontRegion(TextRegion region)
{
    for (auto &[textRegion, enabled] : gRegions) {
        if (textRegion == region) {
            enabled = true;
        }
    }
}

FontResolution *FindFontIndex(int idx)
{
    return &Font;
}

void FontResources::SetFontIndex(int idx)
{
    CurrentFontIndex = idx;

    ImGui::GetIO().FontDefault = GetFont();
}

FontResolution *FontResources::GetFontIndex(int idx)
{
    return FindFontIndex(idx);
}

ImFont *FontResources::GetFont()
{
    FontResolution *font = FindFontIndex(CurrentFontIndex);

    return font ? font->Font : nullptr;
}

ImFont *FontResources::GetButtonFont()
{
    FontResolution *font = FindFontIndex(CurrentFontIndex);

    return font ? font->ButtonFont : nullptr;
}

ImFont *FontResources::GetReallyBigFontForSlider()
{
    FontResolution *font = FindFontIndex(CurrentFontIndex);

    return font ? font->SliderFont : nullptr;
}