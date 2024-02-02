/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __OPENGLBACKEND_H_
#define __OPENGLBACKEND_H_
#include "./glad/gl.h"
#include <Graphics/GraphicsBackendBase.h>
#include <map>
#include <vector>

namespace Graphics {
    namespace Backends {
        struct ShaderData
        {
            GLuint vert;
            GLuint frag;
            GLuint program;
            GLint  location;
        };

        struct OpenGLData
        {
            void                                    *ctx;
            GLuint                                   vertexBuffer;
            GLuint                                   indexBuffer;
            GLuint                                   constantBuffer;
            GLuint                                   maxVertexBufferSize;
            GLuint                                   maxIndexBufferSize;
            std::map<ShaderFragmentType, ShaderData> shaders;
        };

        struct OpenGLDrawItem
        {
            uint32_t           count;
            ShaderFragmentType type;
            BlendHandle        blend;
            const void        *image;

            Rect      clipRect;
            glm::vec2 uiSize;
            glm::vec4 uiRadius;
        };

        struct OpenGLDrawData
        {
            std::vector<Vertex>   vertex;
            std::vector<uint16_t> indices;

            uint32_t vertexSize;
            uint32_t indiceSize;

            uint32_t currentIndexCount;

            void Reset()
            {
                vertexSize = 0;
                indiceSize = 0;
                currentIndexCount = 0;
            }
        };

        class OpenGL : public Base
        {
        public:
            virtual ~OpenGL() = default;

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

            virtual BlendHandle CreateBlendState(TextureBlendInfo blendInfo) override;

            GLuint CreateTexture();
            void   DestroyTexture(GLuint texture);

        private:
            void CreateShader();
            void CreateDefaultBlend();

            void       FlushQueue();
            OpenGLData Data;

            bool m_HasImgui = false;

            OpenGLDrawData                          drawData;
            std::vector<OpenGLDrawItem>             submitInfos;
            std::vector<GLuint>                     textures;
            std::map<BlendHandle, TextureBlendInfo> blendStates;
        };
    } // namespace Backends
} // namespace Graphics

#endif