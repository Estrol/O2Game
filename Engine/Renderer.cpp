#include "Renderer.hpp"
#include "framework.h"
#include <filesystem>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include "Win32ErrorHandling.h"
#include <iostream>

#pragma comment(lib, "dxguid.lib")
//typedef HRESULT(*D3D11_CreateDeviceAndSwapChain)(
//    IDXGIAdapter*               pAdapter,
//    D3D_DRIVER_TYPE             DriverType,
//    HMODULE                     Software,
//    UINT                        Flags,
//    const D3D_FEATURE_LEVEL*    pFeatureLevels,
//    UINT                        FeatureLevels,
//    UINT                        SDKVersion,
//    const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
//    IDXGISwapChain**            ppSwapChain,
//    ID3D11Device**              ppDevice,
//    D3D_FEATURE_LEVEL*          pFeatureLevel,
//    ID3D11DeviceContext**       ppImmediateContext
//);

typedef HRESULT(*DXGI_CreateFactory2)(
    UINT   Flags,
    REFIID riid,
    void** ppFactory
);

typedef HRESULT(*D3D11_CreateDevice)(
    IDXGIAdapter*           pAdapter,
    D3D_DRIVER_TYPE         DriverType,
    HMODULE                 Software,
    UINT                    Flags,
    const D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT                    FeatureLevels,
    UINT                    SDKVersion,
    ID3D11Device**          ppDevice,
    D3D_FEATURE_LEVEL*      pFeatureLevel,
    ID3D11DeviceContext**   ppImmediateContext
);

constexpr auto MAIN_SPRITE_BATCH = 0;

Renderer::Renderer() {
	m_scissorRect = std::unordered_map<int, RECT>();
    m_spriteBatches = std::unordered_map<int, DirectX::SpriteBatch*>();
}


Renderer::~Renderer() {
    Destroy();  
}

Renderer* Renderer::s_instance = nullptr;

bool Renderer::Create(RendererMode mode, Window* window) {
    try {
        HMODULE d3d11 = NULL;
        HMODULE dxgi = NULL;

		std::cout << "DLL LOAD" << std::endl;
        if (mode == RendererMode::VULKAN) {
            std::filesystem::path dxvkPath = std::filesystem::current_path() / "vulkan";

            // This is so cheating!!
            if (std::filesystem::exists(dxvkPath / "dxgi.dll") && std::filesystem::exists(dxvkPath / "d3d11.dll")) {
                dxgi = LoadLibraryA((dxvkPath / "dxgi.dll").string().c_str());
                d3d11 = LoadLibraryA((dxvkPath / "d3d11.dll").string().c_str());
            }
            else {
                MessageBoxA(NULL, "Failed to load Vulkan DLLs", "EstEngine Error", MB_OK);
                return false;
            }
        }
        else {
            dxgi = LoadLibraryExA("dxgi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            d3d11 = LoadLibraryExA("d3d11.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        }

        if (d3d11 == NULL || dxgi == NULL) {
            MessageBoxA(NULL, "Failed to load D3D11 DLLs", "EstEngine Error", MB_ICONERROR);
            return false;
        }

        /*D3D11_CreateDeviceAndSwapChain createDeviceAndSwapChain = (D3D11_CreateDeviceAndSwapChain)GetProcAddress(d3d11, "D3D11CreateDeviceAndSwapChain");
        if (!createDeviceAndSwapChain) {
            MessageBoxA(NULL, "D3D11CreateDeviceAndSwapChain function exports symbols not found in D3D11.dll", "EstEngine Error", MB_ICONERROR);
            return false;
        }*/

		DXGI_CreateFactory2 createFactory = (DXGI_CreateFactory2)GetProcAddress(dxgi, "CreateDXGIFactory2");
		D3D11_CreateDevice createDevice = (D3D11_CreateDevice)GetProcAddress(d3d11, "D3D11CreateDevice");

        if (!createDevice || !createFactory) {
			MessageBoxA(NULL, "D3D11CreateDevice or CreateDXGIFactory2 function exports symbols not found in D3D11.dll or DXGI.dll", "EstEngine Error", MB_ICONERROR);
			return false;
		}

        IDXGIFactory* dxgiFactory;

        UINT factoryFlag = 0;
		
#ifdef _DEBUG
        factoryFlag |= DXGI_CREATE_FACTORY_DEBUG;
#endif // DEBUG

        HRESULT result = createFactory(factoryFlag, IID_PPV_ARGS(&dxgiFactory));
        Win32Exception::ThrowIfError(result, "Failed to create DXGI Factory!");

	    IDXGIAdapter* dxgiAdapter;
        result = dxgiFactory->EnumAdapters(0, &dxgiAdapter);
        Win32Exception::ThrowIfError(result, "Failed to query Graphics Adapter to use!");

        if (!dxgiAdapter) {
            MessageBoxA(NULL, "Error, pDxgiAdapter is NULL", "EstEngine Error", MB_ICONERROR);
            return false;
        }

        HWND handle = window->GetHandle();
        UINT creationFlags = 0;
		
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DEBUG

        const std::vector<D3D_FEATURE_LEVEL> level = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

	    std::cout << "Create Device" << std::endl;
        D3D_FEATURE_LEVEL outLevel = D3D_FEATURE_LEVEL_11_1;

	    result = createDevice(
            dxgiAdapter, 
            D3D_DRIVER_TYPE_UNKNOWN, 
            NULL, 
            creationFlags, 
            level.data(),
            level.size(),
            D3D11_SDK_VERSION, 
            &m_device,
            &outLevel,
            &m_immediateContext
        );
		
        Win32Exception::ThrowIfError(
            result,
            "Failed to create D3D11 Device!"
        );
		
        DXGI_SWAP_CHAIN_DESC scd = { 0 };
        scd.BufferCount = 1;
        scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        scd.BufferDesc.Width = window->GetBufferWidth();
        scd.BufferDesc.Height = window->GetBufferHeight();
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = handle;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;

		result = dxgiFactory->CreateSwapChain(m_device, &scd, &m_swapChain);
        Win32Exception::ThrowIfError(
            result,
            "Failed to create D3D11 SwapChain!"
        );

        ID3D11Texture2D* backBuffer = nullptr;
        result = m_swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backBuffer);
        Win32Exception::ThrowIfError(
            result,
            "Failed to get back buffer"
        );

        if (backBuffer == nullptr) {
			MessageBoxA(NULL, "Error, backBuffer is NULL", "EstEngine Fatal Error", MB_ICONERROR);
			return false;
        }

		std::cout << "Create Render Target View" << std::endl;
        result = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);

        backBuffer->Release();
        Win32Exception::ThrowIfError(
            result,
            "Failed to create render target view"
        );
		
		std::cout << "Create Depth Stencil View" << std::endl;
        m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

        D3D11_VIEWPORT pViewport;
        ZeroMemory(&pViewport, sizeof(D3D11_VIEWPORT));
        pViewport.TopLeftX = 0;
        pViewport.TopLeftY = 0;
        pViewport.Width = window->GetWidth();
        pViewport.Height = window->GetHeight();


		std::cout << "Create Viewport" << std::endl;
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

		std::cout << "Create Rasterizer State" << std::endl;
        result = m_device->CreateRasterizerState(&rsDesc, &m_scissorState);

		Win32Exception::ThrowIfError(
            result, 
            "Failed to create rasterizer state"
        );

        CD3D11_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable = false;
        blendDesc.IndependentBlendEnable = false;
        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		std::cout << "Create Blend State" << std::endl;
		result = m_device->CreateBlendState(&blendDesc, &m_blendState);
		
		Win32Exception::ThrowIfError(
			result,
			"Failed to create blend state"
		);

        m_states = new DirectX::CommonStates(m_device);
        return true;
    }
    catch (Win32Exception& e) {
        MessageBoxA(NULL, e.what(), "EstEngine Error", MB_ICONERROR);
        return false;
    }
}

bool Renderer::Resize() {
    try {
        Window* window = Window::GetInstance();

        HRESULT result = m_swapChain->ResizeBuffers(0, window->GetBufferWidth(), window->GetBufferHeight(), DXGI_FORMAT_UNKNOWN, 0);

        Win32Exception::ThrowIfError(
            result,
            "Failed to resize swap chain buffer size"
        );

        ID3D11Texture2D* backBuffer = nullptr;
        result = m_swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backBuffer);

        Win32Exception::ThrowIfError(
            result,
            "Failed to get back buffer"
        );

        if (backBuffer == nullptr) {
            MessageBoxA(NULL, "Error, backBuffer is NULL", "EstEngine Fatal Error", MB_ICONERROR);
            return false;
        }

        result = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);

        Win32Exception::ThrowIfError(
            result,
            "Failed to create render target view"
        );

        m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

        D3D11_VIEWPORT pViewport;
        ZeroMemory(&pViewport, sizeof(D3D11_VIEWPORT));
        pViewport.TopLeftX = 0;
        pViewport.TopLeftY = 0;
        pViewport.Width = window->GetWidth();
        pViewport.Height = window->GetHeight();

        m_immediateContext->RSSetViewports(1, &pViewport);
        backBuffer->Release();

        return true;
    }
    catch (Win32Exception& e) {
        MessageBoxA(NULL, e.what(), "EstEngine Error", MB_ICONERROR);
        return false;
    }
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

ID3D11BlendState* Renderer::GetBlendState() const {
    return m_blendState;
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

    SAFE_RELEASE(m_scissorState);

    ID3D11RenderTargetView* nullview[] = {nullptr};
	m_immediateContext->OMSetRenderTargets(1, nullview, nullptr);
	
    m_immediateContext->Flush();

    SAFE_RELEASE(m_renderTargetView);
    SAFE_RELEASE(m_immediateContext);
    SAFE_RELEASE(m_swapChain);
    SAFE_RELEASE(m_device);

    return true;
}
