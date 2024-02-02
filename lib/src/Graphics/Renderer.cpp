/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "./Backends/OpenGL/OpenGLBackend.h"
#include "./Backends/OpenGL/OpenGLTexture2D.h"
#include "./Backends/Vulkan/VulkanBackend.h"
#include "./Backends/Vulkan/VulkanTexture2D.h"
#include <Exceptions/EstException.h>
#include <Graphics/Renderer.h>
#include <iostream>
using namespace Graphics;

Renderer *Renderer::s_Instance = nullptr;

Renderer *Renderer::Get()
{
    if (s_Instance == nullptr) {
        s_Instance = new Renderer();
    }
    return s_Instance;
}

void Renderer::Destroy()
{
    if (s_Instance != nullptr) {
        delete s_Instance;
        s_Instance = nullptr;
    }
}

Renderer::~Renderer()
{
    if (m_Backend) {
        m_Backend->Shutdown();
        delete m_Backend;
    }
}

void Renderer::Init(API api, TextureSamplerInfo sampler)
{
    using namespace Backends;

    m_API = api;
    m_Sampler = sampler;

    ::printf("API: %d\n", api);

    Base *backend = nullptr;
    switch (api) {
        case API::Vulkan:
        {
            backend = new Vulkan();
            break;
        }

        case API::OpenGL:
        {
            backend = new OpenGL();
            break;
        }

        default:
        {
            throw Exceptions::EstException("Unknown API");
        }
    }

    backend->Init();

    m_Backend = backend;
}

Backends::Base *Renderer::GetBackend()
{
    return m_Backend;
}

API Renderer::GetAPI()
{
    return m_API;
}

void Renderer::SetVSync(bool vsync)
{
    m_Backend->SetVSync(vsync);
}

void Renderer::Push(Graphics::Backends::SubmitInfo &info)
{
    m_Backend->Push(info);
}

void Renderer::Push(std::vector<Graphics::Backends::SubmitInfo> &infos)
{
    m_Backend->Push(infos);
}

bool Renderer::BeginFrame()
{
    if (!m_Backend) {
        throw Exceptions::EstException("Renderer backend not initialized");
    }

    if (m_onFrame) {
        throw Exceptions::EstException("BeginFrame called without EndFrame");
    }

    if (m_Backend->NeedReinit()) {
        m_Backend->ReInit();
    }

    auto result = m_Backend->BeginFrame();
    m_onFrame = result;
    return result;
}

void Renderer::EndFrame()
{
    if (!m_Backend) {
        throw Exceptions::EstException("Renderer backend not initialized");
    }

    if (!m_onFrame) {
        throw Exceptions::EstException("EndFrame called without BeginFrame");
    }

    m_onFrame = false;
    m_Backend->EndFrame();
}

void Renderer::ImGui_NewFrame()
{
    if (!m_Backend) {
        throw Exceptions::EstException("Renderer backend not initialized");
    }

    m_Backend->ImGui_NewFrame();
}

void Renderer::ImGui_EndFrame()
{
    if (!m_Backend) {
        throw Exceptions::EstException("Renderer backend not initialized");
    }

    m_Backend->ImGui_EndFrame();
}

Texture2D *CreateTexture(API api, TextureSamplerInfo sampler)
{
    Texture2D *texture = nullptr;

    switch (api) {
        case API::Vulkan:
        {
            texture = new VKTexture2D(sampler);
            break;
        }

        case API::OpenGL:
        {
            texture = new GLTexture2D(sampler);
            break;
        }
    }

    return texture;
}

Texture2D *Renderer::LoadTexture(std::filesystem::path path)
{
    auto texture = CreateTexture(GetAPI(), m_Sampler);

    texture->Load(path);

    return texture;
}

Texture2D *Renderer::LoadTexture(const char *buf, size_t size)
{
    auto texture = CreateTexture(GetAPI(), m_Sampler);

    texture->Load(buf, size);

    return texture;
}

Texture2D *Renderer::LoadTexture(const char *pixbuf, uint32_t width, uint32_t height)
{
    auto texture = CreateTexture(GetAPI(), m_Sampler);

    texture->Load(pixbuf, width, height);

    return texture;
}

Graphics::Backends::BlendHandle Renderer::CreateBlendState(Graphics::Backends::TextureBlendInfo info)
{
    return m_Backend->CreateBlendState(info);
}