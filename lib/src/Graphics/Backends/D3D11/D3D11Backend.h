/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __D3D11BACKEND_H_
#define __D3D11BACKEND_H_

#if _WIN32 && defined(__ENABLE_D3D11__)
#ifndef _MSC_VER
#error "This complication only support MSVC compiler"
#endif

#include <Graphics/GraphicsBackendBase.h>
#include <d3d11.h>
#include <map>
#include <vector>

namespace Graphics {
    namespace Backends {
        struct D3D11Object
        {
            ID3D11Device        *dev;
            ID3D11DeviceContext *devcon;
            IDXGISwapChain      *swapchain;

            bool                    VSync;
            float                   clearColor[4];
            ID3D11RenderTargetView *renderTargetView;
            ID3D11RasterizerState  *rasterizerState;
        };

        class D3D11 : public Base
        {
        public:
            virtual ~D3D11();

            virtual void Init() override;
            virtual void ReInit() override;
            virtual void Shutdown() override;

            virtual bool NeedReinit() override;

            virtual bool BeginFrame() override;
            virtual void EndFrame() override;

            virtual void ImGui_Init() override;
            virtual void ImGui_DeInit() override;
            virtual void ImGui_NewFrame() override;
            virtual void ImGui_EndFrame() override;

            virtual void Push(SubmitInfo &info) override;
            virtual void Push(std::vector<SubmitInfo> &infos) override;

            virtual void SetVSync(bool enabled) override;
            virtual void SetClearColor(glm::vec4 color) override;
            virtual void SetClearDepth(float depth) override;
            virtual void SetClearStencil(uint32_t stencil) override;

            virtual void CaptureFrame(std::function<void(std::vector<unsigned char>)>) override;

            virtual BlendHandle CreateBlendState(TextureBlendInfo blendInfo) override;

            D3D11Object *GetData();

        private:
            void FlushQueue();

            D3D11Object Data;
        };
    } // namespace Backends
} // namespace Graphics

#endif
#endif