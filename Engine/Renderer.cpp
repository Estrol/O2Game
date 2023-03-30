#include "Renderer.hpp"
#include "framework.h"
#include <filesystem>
#include <d3d11.h>
#include <wrl.h>

#pragma comment(lib, "dxguid.lib")
typedef HRESULT(*D3D11_CreateDeviceAndSwapChain)(
    IDXGIAdapter*               pAdapter,
    D3D_DRIVER_TYPE             DriverType,
    HMODULE                     Software,
    UINT                        Flags,
    const D3D_FEATURE_LEVEL*    pFeatureLevels,
    UINT                        FeatureLevels,
    UINT                        SDKVersion,
    const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
    IDXGISwapChain**            ppSwapChain,
    ID3D11Device**              ppDevice,
    D3D_FEATURE_LEVEL*          pFeatureLevel,
    ID3D11DeviceContext**       ppImmediateContext
);

#define MAIN_SPRITE_BATCH 0

Renderer::Renderer() {
	m_scissorRect = std::unordered_map<int, RECT>();
    m_spriteBatches = std::unordered_map<int, DirectX::SpriteBatch*>();
}

Renderer::~Renderer() {
    Destroy();  
}

Renderer* Renderer::s_instance = nullptr;

bool Renderer::Create(RendererMode mode, Window* window, RECT& internalResolution) {
    HMODULE d3d11 = NULL;

    if (mode == RendererMode::VULKAN) {
		std::filesystem::path dxvkPath = std::filesystem::current_path() / "vulkan";

		// This is so cheating!!
        if (std::filesystem::exists(dxvkPath / "dxgi.dll") && std::filesystem::exists(dxvkPath / "d3d11.dll")) {
            LoadLibraryA((dxvkPath / "dxgi.dll").string().c_str());
            d3d11 = LoadLibraryA((dxvkPath / "d3d11.dll").string().c_str());
        }
        else {
			MessageBoxA(NULL, "Failed to load Vulkan DLLs", "EstEngine Error", MB_OK);
            return false;
        }
    }
    else {
        d3d11 = LoadLibraryA("d3d11.dll");
    }

	if (d3d11 == NULL) {
		MessageBoxA(NULL, "Failed to load D3D11 DLLs", "EstEngine Error", MB_ICONERROR);
		return false;
	}

	D3D11_CreateDeviceAndSwapChain createDeviceAndSwapChain = (D3D11_CreateDeviceAndSwapChain)GetProcAddress(d3d11, "D3D11CreateDeviceAndSwapChain");
    if (!createDeviceAndSwapChain) {
		MessageBoxA(NULL, "D3D11CreateDeviceAndSwapChain function exports symbols not found in D3D11.dll", "EstEngine Error", MB_ICONERROR);
		return false;
    }
	
	HWND handle = window->GetHandle();
	
    DXGI_SWAP_CHAIN_DESC scd = { 0 };
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = internalResolution.right;
    scd.BufferDesc.Height = internalResolution.bottom;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = handle;
    scd.SampleDesc.Count = 4;
    scd.Windowed = TRUE;

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;

    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;
    D3D_FEATURE_LEVEL level2 = D3D_FEATURE_LEVEL_11_1;

	HRESULT result = createDeviceAndSwapChain(
        nullptr, 
        D3D_DRIVER_TYPE_HARDWARE, 
        nullptr, 
        creationFlags, 
        &level, 
        1, 
        D3D11_SDK_VERSION, 
        &scd, 
        &m_swapChain, 
        &m_device, 
        &level2, 
        &m_immediateContext
    );

	if (FAILED(result)) {
		MessageBoxA(NULL, "Failed to create device and swap chain", "EstEngine Error", MB_ICONERROR);
		return false;
	}

	ID3D11Texture2D* backBuffer = nullptr;
	result = m_swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backBuffer);

    if (FAILED(result)) {
        MessageBoxA(NULL, "Failed to get back buffer", "EstEngine Error", MB_ICONERROR);
        return false;
    }

	result = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
    backBuffer->Release();

	if (FAILED(result)) {
		MessageBoxA(NULL, "Failed to create render target view", "EstEngine Error", MB_ICONERROR);
		return false;
	}

	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

    D3D11_VIEWPORT pViewport;
    ZeroMemory(&pViewport, sizeof(D3D11_VIEWPORT));
    pViewport.TopLeftX = 0;
    pViewport.TopLeftY = 0;
    pViewport.Width = window->GetWidth();
    pViewport.Height = window->GetHeight();

    m_immediateContext->RSSetViewports(1, &pViewport);
	
    CD3D11_RASTERIZER_DESC rsDesc(
        D3D11_FILL_SOLID, 
        D3D11_CULL_BACK, 
        FALSE,
        0, 
        0.f, 
        0.f, 
        TRUE, 
        TRUE, 
        TRUE, 
        FALSE
    );

	result = m_device->CreateRasterizerState(&rsDesc, &m_scissorState);
	if (FAILED(result)) {
		MessageBoxA(NULL, "Failed to create rasterizer state", "EstEngine Error", MB_ICONERROR);
		return false;
	}

    m_states = new DirectX::CommonStates(m_device);
    return true;
}

bool Renderer::BeginRender() {
    float colors[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_immediateContext->ClearRenderTargetView(m_renderTargetView, colors);
	
	return true;
}

bool Renderer::EndRender() {
	m_swapChain->Present(0, 0);
	
	return true;
}

bool Renderer::CreateScissor(RECT rect) {
	int id = rect.left * 1.2 + rect.top * 1.3 + rect.right * 1.4 + rect.bottom * 1.5;
    if (m_scissorRect.find(id) != m_scissorRect.end()) {
        return true;
    }

	m_scissorRect[id] = rect;

    return true;
}

ID3D11Device* Renderer::GetDevice() const {
    return m_device;
}

ID3D11DeviceContext* Renderer::GetImmediateContext() const {
    return m_immediateContext;
}

IDXGISwapChain* Renderer::GetSwapChain() const {
    return m_swapChain;
}

ID3D11RasterizerState* Renderer::GetRasterizerState() const {
    return m_scissorState;
}

DirectX::SpriteBatch* Renderer::GetSpriteBatch() {
    return GetSpriteBatch(MAIN_SPRITE_BATCH);
}

DirectX::SpriteBatch* Renderer::GetSpriteBatch(int index) {
    if (index < 0 || index >= 100) {
        throw std::out_of_range("Expected index was between 0-99");
    }

    if (m_spriteBatches[index] == nullptr) {
        m_spriteBatches[index] = new DirectX::SpriteBatch(m_immediateContext);
    }

    return m_spriteBatches[index];
}

DirectX::CommonStates* Renderer::GetStates() {
    return m_states;
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
    for (auto& batch : m_spriteBatches) {
        delete batch.second;
    }

    if (m_states) {
        delete m_states;
    }
	
    if (m_renderTargetView) {
        m_renderTargetView->Release();
    }

    if (m_immediateContext) {
        m_immediateContext->Release();
    }

    if (m_swapChain) {
        m_swapChain->Release();
    }

	if (m_device) {
		m_device->Release();
	}

    return true;
}
