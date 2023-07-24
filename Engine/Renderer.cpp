#include "Renderer.hpp"
#include "framework.h"
#include <filesystem>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3d11on12.h>
#include <dxgidebug.h>
#include <wrl.h>
#include "Win32ErrorHandling.h"
#include "Imgui/imgui_impl_sdl2.h"
#include "Imgui/imgui_impl_sdlrenderer2.h"
#include "Imgui/ImguiUtil.hpp"
#include "Imgui/implot.h"
#include <iostream>
#include "SDLException.hpp"
#include "Data/SDLRenderStruct.h"
#include "VulkanDriver/VulkanEngine.h"

constexpr auto MAIN_SPRITE_BATCH = 0;

Renderer::Renderer() {
    m_blendMode = SDL_BLENDMODE_NONE;
    m_renderer = NULL;
}


Renderer::~Renderer() {
    Destroy();  
}

Renderer* Renderer::s_instance = nullptr;

bool Renderer::Create(RendererMode mode, Window* window, bool failed) {
    try {
        std::string rendererName = "";
        bool bUsedSDLRenderer = true;

        switch (mode) {
            case RendererMode::OPENGL: {
                rendererName = "opengl";
                break;
            }

            case RendererMode::VULKAN: {
                bUsedSDLRenderer = false;
                break;
            }

            case RendererMode::DIRECTX: {
                rendererName = "direct3d";
                break;
            }

            case RendererMode::DIRECTX11: {
                rendererName = "direct3d11";
                break;
            }

            case RendererMode::DIRECTX12: {
                rendererName = "direct3d12";
                break;
            }

            case RendererMode::METAL: {
				rendererName = "metal";
				break;
            }
        }

        if (bUsedSDLRenderer) {
            // loop and find the first available renderer
            if (!failed) {
                bool found = false;
                int numRenderers = SDL_GetNumRenderDrivers();
                for (int i = 0; i < numRenderers; i++) {
                    SDL_RendererInfo info;
                    SDL_GetRenderDriverInfo(i, &info);

                    if (info.flags & SDL_RENDERER_SOFTWARE) {
                        continue;
                    }

                    if (rendererName == info.name) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    // fallback
                    return Create(mode, window, true);
                }

                SDL_SetHint(SDL_HINT_RENDER_DRIVER, rendererName.c_str());
                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Fix Aliasing
            }

            m_renderer = SDL_CreateRenderer(window->GetWindow(), -1, SDL_RENDERER_ACCELERATED);
            if (!m_renderer) {
                throw SDLException();
            }

            if (failed) {
                std::cout << "[Renderer] Failed to create renderer: " << rendererName << " and fallback to " << SDL_GetCurrentVideoDriver() << std::endl;
            }

            m_blendMode = SDL_ComposeCustomBlendMode(
                SDL_BLENDFACTOR_SRC_ALPHA,
                SDL_BLENDFACTOR_ONE,
                SDL_BLENDOPERATION_ADD,
                SDL_BLENDFACTOR_ONE,
                SDL_BLENDFACTOR_ZERO,
                SDL_BLENDOPERATION_ADD);

            if (SDL_SetRenderDrawBlendMode(m_renderer, m_blendMode) == -1) {
                throw SDLException();
            }

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImPlot::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); (void)io;

            // Manually set the display size
            io.DisplaySize.x = (float)window->GetWidth();
            io.DisplaySize.y = (float)window->GetHeight();
            io.DisplayOutputSize.x = (float)window->GetWidth();
            io.DisplayOutputSize.y = (float)window->GetHeight();
            io.IniFilename = NULL;
            io.WantSaveIniSettings = false;

            ImGui::StyleColorsDark();

            if (!ImGui_ImplSDL2_InitForSDLRenderer(window->GetWindow(), m_renderer)) {
                throw SDLException();
            }

            if (!ImGui_ImplSDLRenderer2_Init(m_renderer)) {
                throw SDLException();
            }

            return true;
        }
        else {
            m_vulkan = VulkanEngine::GetInstance();
			m_vulkan->init(window->GetWindow(), window->GetWidth(), window->GetHeight());

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImguiUtil::SetVulkan(true);

            // Manually set the display size
            io.DisplaySize.x = (float)window->GetWidth();
            io.DisplaySize.y = (float)window->GetHeight();
            io.DisplayOutputSize.x = (float)window->GetWidth();
            io.DisplayOutputSize.y = (float)window->GetHeight();
            io.IniFilename = NULL;
            io.WantSaveIniSettings = false;

            return true;
        }
    }
	catch (SDLException& e) {
		MessageBoxA(NULL, e.what(), "EstEngine Error", MB_ICONERROR);
		return false;
	}
}

bool Renderer::Resize() {
    try {
        Window* window = Window::GetInstance();

        int new_width = window->GetWidth();
        int new_height = window->GetHeight();

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Manually set the display size
        io.DisplaySize.x = (float)new_width;
        io.DisplaySize.y = (float)new_height;
        io.DisplayOutputSize.x = (float)new_width;
        io.DisplayOutputSize.y = (float)new_height;

        if (!IsVulkan()) {
            if (SDL_RenderSetLogicalSize(m_renderer, new_width, new_height) == -1) {
                throw SDLException();
            }
        }

        return true;
    }
    catch (SDLException& e) {
        MessageBoxA(NULL, e.what(), "EstEngine Error", MB_ICONERROR);
        return false;
    }
}

bool Renderer::BeginRender() {
	// sdl clear color
    if (IsVulkan()) {
        if (m_vulkan->_swapChainOutdated) {
            return false;
        }

        m_vulkan->begin();
    }
    else {
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);
    }

	return true;
}

bool Renderer::EndRender() {
    if (IsVulkan()) {
        m_vulkan->flush_queue();

        if (ImguiUtil::HasFrameQueue()) {
            ImguiUtil::Reset();
        }

        m_vulkan->end();
    }
    else {
        if (ImguiUtil::HasFrameQueue()) {
            ImguiUtil::Reset();

            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        }

        SDL_RenderPresent(m_renderer);
    }

	return true;
}

void Renderer::ResetImGui() {

}

SDL_Renderer* Renderer::GetSDLRenderer() {
    return m_renderer;
}

SDL_BlendMode Renderer::GetSDLBlendMode() {
    return m_blendMode;
}

VulkanEngine* Renderer::GetVulkanEngine() {
    return m_vulkan;
}

bool Renderer::IsVulkan() {
    return GetVulkanEngine() != nullptr;
}

Renderer* Renderer::GetInstance() {
	if (s_instance == nullptr) {
		s_instance = new Renderer();
	}

	return s_instance;
}

void Renderer::Release() {
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
	}
}

RendererMode Renderer::GetBestRendererMode() {
#if _WIN32
    return RendererMode::DIRECTX11; // I hope, all windows user support this
#elif __APPLE__
	return RendererMode::METAL; // Not sure if this work
#else
	return RendererMode::OPENGL; // Also I'm not sure if linux users support vulkan
#endif
}

bool Renderer::Destroy() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }

    if (m_vulkan) {
        VulkanEngine::Release();
    }

    return true;
}
