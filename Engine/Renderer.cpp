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
#include "Imgui/imgui_impl_dx11.h"
#include "Imgui/imgui_impl_sdlrenderer2.h"
#include "Imgui/ImguiUtil.hpp"
#include <iostream>
#include "SDLException.hpp"
#include "Data/SDLRenderStruct.h"

#pragma comment(lib, "dxguid.lib")
constexpr auto MAIN_SPRITE_BATCH = 0;

Renderer::Renderer() {
    m_blendMode = SDL_BLENDMODE_NONE;
    m_renderer = NULL;
}


Renderer::~Renderer() {
    Destroy();  
}

Renderer* Renderer::s_instance = nullptr;

bool Renderer::Create(RendererMode mode, Window* window) {
    try {
        std::string rendererName = "";

        switch (mode) {
            case RendererMode::OPENGL: {
                // set renderer hint to opengl
                rendererName = "opengl";
                break;
            }
									
            case RendererMode::VULKAN: {
                rendererName = "direct3d";
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
        }

		// loop and find the first available renderer
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
			throw std::runtime_error("Renderer " + rendererName + " is not supported in this system!");
        }

        if (mode == RendererMode::VULKAN) {
            std::filesystem::path dxvkPath = std::filesystem::current_path() / "vulkan";

            if (LoadLibraryA((dxvkPath / "d3d9.dll").string().c_str()) == NULL) {
                throw std::runtime_error("Failed to load DXVK (D3D9.dll) library");
            }
        }

		SDL_SetHint(SDL_HINT_RENDER_DRIVER, rendererName.c_str());
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
        m_renderer = SDL_CreateRenderer(window->GetWindow(), -1, SDL_RENDERER_ACCELERATED);
        if (!m_renderer) {
            throw SDLException();
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
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Manually set the display size
        io.DisplaySize.x = window->GetWidth();
        io.DisplaySize.y = window->GetHeight();
        io.DisplayOutputSize.x = window->GetWidth();
        io.DisplayOutputSize.y = window->GetHeight();
        io.IniFilename = NULL;
        io.WantSaveIniSettings = false;

        ImGui::StyleColorsDark();

        if (!ImGui_ImplSDL2_InitForSDLRenderer(window->GetWindow(), m_renderer)) {
            throw SDLException();
        }

        if (!ImGui_ImplSDLRenderer2_Init(m_renderer)) {
            throw SDLException();
        }

        //SDL_SetWindowFullscreen(window->GetWindow(), SDL_WINDOW_FULLSCREEN);

        return true;
    }
    catch (Win32Exception& e) {
        MessageBoxA(NULL, e.what(), "EstEngine Error", MB_ICONERROR);
        return false;
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

        // resize SDL_Renderer
		if (SDL_RenderSetLogicalSize(m_renderer, new_width, new_height) == -1) {
			throw SDLException();
		}

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Manually set the display size
        io.DisplaySize.x = new_width;
        io.DisplaySize.y = new_height;
        io.DisplayOutputSize.x = new_width;
        io.DisplayOutputSize.y = new_height;

        return true;
    }
    catch (SDLException& e) {
        MessageBoxA(NULL, e.what(), "EstEngine Error", MB_ICONERROR);
        return false;
    }
}

bool Renderer::BeginRender() {
	// sdl clear color
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_renderer);

	return true;
}

bool Renderer::EndRender() {
    SDL_RenderPresent(m_renderer);
	return true;
}

SDL_Renderer* Renderer::GetSDLRenderer() {
    return m_renderer;
}

SDL_BlendMode Renderer::GetSDLBlendMode() {
    return m_blendMode;
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

bool Renderer::Destroy() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }

    return true;
}
