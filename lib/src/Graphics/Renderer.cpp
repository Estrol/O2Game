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
        m_textures_caches.clear();

        m_Backend->Shutdown();
        m_Backend.reset();
    }
}

void Renderer::Init(API api, TextureSamplerInfo sampler)
{
    using namespace Backends;

    m_API = api;
    m_Sampler = sampler;

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

    m_Backend = std::shared_ptr<Base>(backend);
}

Backends::Base *Renderer::GetBackend()
{
    return m_Backend.get();
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

std::string HashTexturePath(std::filesystem::path path)
{
    std::hash<std::string> hasher;
    return std::to_string(hasher(path.string()));
}

std::string HashTextureMemory(const unsigned char *buf, size_t size)
{
    std::hash<std::string> hasher;
    return std::to_string(hasher(std::string((const char *)buf, size)));
}

std::shared_ptr<Texture2D> CreateTexture(API api, TextureSamplerInfo sampler)
{
    std::shared_ptr<Texture2D> texture;

    switch (api) {
        case API::Vulkan:
        {
            auto texvk = new VKTexture2D(sampler);
            texture = std::shared_ptr<Texture2D>(texvk);
            break;
        }

        case API::OpenGL:
        {
            auto texgl = new GLTexture2D(sampler);
            texture = std::shared_ptr<Texture2D>(texgl);
            break;
        }
    }

    return texture;
}

std::shared_ptr<Texture2D> Renderer::LoadTexture(std::filesystem::path path)
{
    auto hashKey = HashTexturePath(path);

    if (m_textures_caches.find(hashKey) != m_textures_caches.end()) {
        return m_textures_caches[hashKey];
    }

    auto texture = CreateTexture(GetAPI(), m_Sampler);
    texture->Load(path);

    m_textures_caches[hashKey] = texture;
    return texture;
}

std::shared_ptr<Texture2D> Renderer::LoadTexture(const unsigned char *buf, size_t size)
{
    auto hashKey = HashTextureMemory(buf, size);

    if (m_textures_caches.find(hashKey) != m_textures_caches.end()) {
        return m_textures_caches[hashKey];
    }

    auto texture = CreateTexture(GetAPI(), m_Sampler);
    texture->Load(buf, size);

    m_textures_caches[hashKey] = texture;
    return texture;
}

std::shared_ptr<Texture2D> Renderer::LoadTexture(const unsigned char *pixbuf, uint32_t width, uint32_t height)
{
    auto hashKey = HashTextureMemory(pixbuf, width * height * 4);

    if (m_textures_caches.find(hashKey) != m_textures_caches.end()) {
        return m_textures_caches[hashKey];
    }

    auto texture = CreateTexture(GetAPI(), m_Sampler);
    texture->Load(pixbuf, width, height);

    m_textures_caches[hashKey] = texture;
    return texture;
}

void Renderer::CaptureFrame(std::function<void(std::vector<unsigned char>)> callback)
{
    return m_Backend->CaptureFrame(callback);
}

Graphics::Backends::BlendHandle Renderer::CreateBlendState(Graphics::Backends::TextureBlendInfo info)
{
    return m_Backend->CreateBlendState(info);
}