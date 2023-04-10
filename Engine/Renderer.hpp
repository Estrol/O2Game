#pragma once
#include "framework.h"
#include "Window.hpp"
#include <unordered_map>
#include <d3d11.h>

enum class RendererMode {
	DIRECTX,
	VULKAN
};

class Renderer {
public:
	bool Create(RendererMode mode, Window* window);
	bool Resize();
	bool Destroy();

	bool BeginRender();
	bool EndRender();

	bool CreateScissor(RECT rect);

	ID3D11Device* GetDevice() const;
	ID3D11DeviceContext* GetImmediateContext() const;
	IDXGISwapChain* GetSwapChain() const;
	ID3D11RasterizerState* GetRasterizerState() const;
	
	DirectX::SpriteBatch* GetSpriteBatch();
	DirectX::SpriteBatch* GetSpriteBatch(int index);
	DirectX::CommonStates* GetStates();

	static Renderer* GetInstance();
	static void Release();

private:
	Renderer();
	~Renderer();

	static Renderer* s_instance;

	ID3D11RasterizerState* m_scissorState;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_immediateContext = nullptr;
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	DirectX::CommonStates* m_states = nullptr;

	std::unordered_map<int, RECT> m_scissorRect;
	std::unordered_map<int, DirectX::SpriteBatch*> m_spriteBatches;
};