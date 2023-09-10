#pragma once
#include "Window.h"
#include <SDL.h>
#undef main

#include <unordered_map>

class VulkanEngine;

enum class RendererMode {
	OPENGL, // 0
	VULKAN, // 1
	DIRECTX, // 2
	DIRECTX11, // 3
	DIRECTX12, // 4
	METAL, // 5
};

class Renderer {
#if _DEBUG
	const char SIGNATURE[25] = "Renderer";
#endif

public:
	bool Create(RendererMode mode, GameWindow* window, bool failed = false);
	bool Resize();
	bool Destroy();

	bool BeginRender();
	bool EndRender();

	void ResetImGui();

	SDL_Renderer* GetSDLRenderer();
	SDL_BlendMode GetSDLBlendMode();

	VulkanEngine* GetVulkanEngine();
	bool IsVulkan();

	static Renderer* GetInstance();
	static void Release();

	static RendererMode GetBestRendererMode();

private:
	Renderer();
	~Renderer();

	static Renderer* s_instance;
	
	SDL_Renderer* m_renderer = nullptr; /* May be used with DirectX11, DirectX12 or OpenGL */
	SDL_BlendMode m_blendMode;
	VulkanEngine* m_vulkan = nullptr;
};