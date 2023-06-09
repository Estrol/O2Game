#pragma once
#include "framework.h"
#include "Window.hpp"
#include <unordered_map>
#include <d3d11.h>

enum class RendererMode {
	OPENGL, // 0
	VULKAN, // 1
	DIRECTX, // 2
	DIRECTX11, // 3
	DIRECTX12, // 4
};

class Renderer {
public:
	bool Create(RendererMode mode, Window* window);
	bool Resize();
	bool Destroy();

	bool BeginRender();
	bool EndRender();

	SDL_Renderer* GetSDLRenderer();
	SDL_BlendMode GetSDLBlendMode();

	static Renderer* GetInstance();
	static void Release();

private:
	Renderer();
	~Renderer();

	static Renderer* s_instance;
	SDL_Renderer* m_renderer; /* May be used with DirectX11, DirectX12 or OpenGL */
	SDL_BlendMode m_blendMode;
};