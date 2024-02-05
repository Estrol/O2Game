/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "D3D11Backend.h"

#if defined(__ENABLE_D3D11__)

#include "Utils/Helper.h"
#include <Exceptions/EstException.h>
#include <Graphics/NativeWindow.h>
#include <SDl2/SDL_syswm.h>

#include "../../Shaders/SPV/image.spv.h"
#include "../../Shaders/SPV/position.spv.h"

// #include <spirv_cross/spirv_hlsl.hpp>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

// std::string compileSPRIV(const uint32_t *data, size_t size)
// {
//     spirv_cross::CompilerHLSL compiler(data, size);

//     spirv_cross::CompilerHLSL::Options options;
//     options.shader_model = 50;

//     compiler.set_hlsl_options(options);

//     return compiler.compile();
// }

using namespace Graphics::Backends;

D3D11::~D3D11()
{
}

void D3D11::Init()
{
    SDL_Window *wnd = Graphics::NativeWindow::Get()->GetWindow();

    Rect windowSize = Graphics::NativeWindow::Get()->GetWindowSize();

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (!SDL_GetWindowWMInfo(wnd, &info)) {
        throw Exceptions::EstException("Failed to init D3D11: Failed to get window context");
    }

    HWND hwnd = info.info.win.window;

    DXGI_SWAP_CHAIN_DESC scd;
    ESTZEROMEMORY(scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = windowSize.Width;
    scd.BufferDesc.Height = windowSize.Height;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 4;
    scd.Windowed = TRUE;

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;
    D3D_FEATURE_LEVEL level2 = D3D_FEATURE_LEVEL_11_1;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        creationFlags,
        &level,
        1,
        D3D11_SDK_VERSION,
        &scd,
        &Data.swapchain,
        &Data.dev,
        &level2,
        &Data.devcon);

    CHECKERROR(hr, "Failed to init D3D11: Failed to create device and swap chain");

    ID3D11Texture2D *backBuffer = nullptr;
    hr = Data.swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&backBuffer);

    CHECKERROR(hr, "Failed to init D3D11: Failed to get back buffer");

    hr = Data.dev->CreateRenderTargetView(backBuffer, NULL, &Data.renderTargetView);

    CHECKERROR(hr, "Failed to init D3D11: Failed to create render target view");

    backBuffer->Release();

    Data.devcon->OMSetRenderTargets(1, &Data.renderTargetView, NULL);

    D3D11_VIEWPORT viewport;
    ESTZEROMEMORY(viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)windowSize.Width;
    viewport.Height = (float)windowSize.Height;

    Data.devcon->RSSetViewports(1, &viewport);

    D3D11_RASTERIZER_DESC rasterizerDesc;
    ESTZEROMEMORY(rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = TRUE;
    rasterizerDesc.MultisampleEnable = TRUE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    hr = Data.dev->CreateRasterizerState(&rasterizerDesc, &Data.rasterizerState);

    CHECKERROR(hr, "Failed to init D3D11: Failed to create rasterizer state");

    ImGui_Init();
}

void D3D11::ReInit() {}

bool D3D11::NeedReinit()
{
    return true;
}

bool D3D11::BeginFrame()
{
    Data.devcon->ClearRenderTargetView(Data.renderTargetView, Data.clearColor);

    return true;
}

void D3D11::EndFrame()
{
    Data.swapchain->Present(Data.VSync ? 1 : 0, 0);
}

void D3D11::SetVSync(bool enabled)
{
    Data.VSync = enabled;
}

void D3D11::SetClearColor(glm::vec4 color)
{
    Data.clearColor[0] = color.r;
    Data.clearColor[1] = color.g;
    Data.clearColor[2] = color.b;
    Data.clearColor[3] = color.a;
}

void D3D11::SetClearDepth(float depth)
{
}

void D3D11::SetClearStencil(uint32_t stencil)
{
}

void D3D11::Push(SubmitInfo &info)
{
}

void D3D11::Push(std::vector<SubmitInfo> &infos)
{
}

void D3D11::FlushQueue()
{
}

#endif