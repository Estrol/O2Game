/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "GraphicsBackendBase.h"
#include "GraphicsTexture2D.h"
#include <map>
#include <string>
#include <vector>

namespace Graphics {
    enum class API {
        None = 0,
        OpenGL = 1,
        Vulkan = 2,
        D3D11 = 3
    };

    class Renderer
    {
    public:
        void Init(API api, TextureSamplerInfo sampler);

        void SetVSync(bool enabled);

        bool BeginFrame();
        void EndFrame();

        void ImGui_NewFrame();
        void ImGui_EndFrame();

        void Push(Graphics::Backends::SubmitInfo &info);
        void Push(std::vector<Graphics::Backends::SubmitInfo> &infos);

        Backends::Base *GetBackend();
        API             GetAPI();

        /*
            Texture handler
            Internal only, you have handle the lifetime of the texture yourself
            Make sure delete the texture before Renderer is destroyed
        */

        std::shared_ptr<Texture2D> LoadTexture(std::filesystem::path path);
        std::shared_ptr<Texture2D> LoadTexture(const unsigned char *buf, size_t size);
        std::shared_ptr<Texture2D> LoadTexture(const unsigned char *pixbuf, uint32_t width, uint32_t height);

        Graphics::Backends::BlendHandle CreateBlendState(Graphics::Backends::TextureBlendInfo info);

        void CaptureFrame(std::function<void(std::vector<unsigned char>)> callback);

        static Renderer *Get();
        static void      Destroy();

    private:
        static Renderer *s_Instance;

        ~Renderer();

        API                m_API;
        TextureSamplerInfo m_Sampler;

        std::shared_ptr<Backends::Base> m_Backend;
        bool                            m_onFrame = false;

        std::map<std::string, std::shared_ptr<Texture2D>> m_textures_caches;
    };
} // namespace Graphics

#endif // __RENDERER_H__