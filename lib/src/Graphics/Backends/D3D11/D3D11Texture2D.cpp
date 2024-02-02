/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "D3D11Texture2D.h"
#include "D3D11Backend.h"
#include "Utils/Helper.h"
#include <Exceptions/EstException.h>
#include <Graphics/Renderer.h>
#include <Graphics/Utils/stb_image.h>
#include <Misc/Filesystem.h>

using namespace Graphics;
using namespace Exceptions;

D3D11Texture2D::D3D11Texture2D(TextureSamplerInfo samplerInfo)
    : Texture2D(samplerInfo)
{
}

D3D11Texture2D::~D3D11Texture2D()
{
    Texture->Release();
    TextureView->Release();
    SamplerState->Release();
}

void D3D11Texture2D::Load(std::filesystem::path path)
{
    Path = path;

    auto data = Misc::Filesystem::ReadFile(path);

    Load((const char *)data.data(), data.size());
}

void D3D11Texture2D::Load(const char *buf, size_t size)
{
    if (Texture != nullptr) {
        throw Exceptions::EstException("Texture already loaded");
    }

    if (buf == nullptr) {
        throw Exceptions::EstException("Invalid buffer");
    }

    auto renderer = Graphics::Renderer::Get();
    if (renderer->GetAPI() != Graphics::API::D3D11) {
        throw EstException("Cannot load D3D11 texture from non-D3D11 renderer");
    }

    auto backend = (Graphics::Backends::D3D11 *)renderer->GetBackend();

    unsigned char *image_data = stbi_load_from_memory(
        (const unsigned char *)buf,
        (int)size,
        &Size.Width,
        &Size.Height,
        &Channels,
        STBI_rgb_alpha);

    if (image_data == nullptr) {
        throw Exceptions::EstException("Failed to load texture: %s", stbi_failure_reason());
    }

    D3D11_TEXTURE2D_DESC desc;
    ESTZEROMEMORY(desc, sizeof(desc));

    desc.Width = Size.Width;
    desc.Height = Size.Height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 4;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data;
    ESTZEROMEMORY(data, sizeof(data));

    data.pSysMem = image_data;
    data.SysMemPitch = Size.Width * 4;

    HRESULT hr = backend->GetData()->dev->CreateTexture2D(&desc, &data, &Texture);

    CHECKERROR(hr, "Failed to create texture");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ESTZEROMEMORY(srvDesc, sizeof(srvDesc));

    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;

    hr = backend->GetData()->dev->CreateShaderResourceView(Texture, &srvDesc, &TextureView);

    CHECKERROR(hr, "Failed to create shader resource view");
}