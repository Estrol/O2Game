#ifndef __GRAPHICSBACKENDBASE_H_
#define __GRAPHICSBACKENDBASE_H_

#include "Utils/Rect.h"
#include <glm/glm.hpp>
#include <vector>

#define MY_OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE *)0)->ELEMENT))

namespace Graphics {
    namespace Backends {
        struct Vertex
        {
            glm::vec2 pos;
            glm::vec2 texCoord;
            uint32_t  color;

            inline void SetColorFloat(glm::vec4 vec4color)
            {
                auto r = vec4color.r;
                auto g = vec4color.g;
                auto b = vec4color.b;
                auto a = vec4color.a;

                color = ((uint32_t)(a * 255.0f) << 24) | ((uint32_t)(b * 255.0f) << 16) | ((uint32_t)(g * 255.0f) << 8) | ((uint32_t)(r * 255.0f) << 0);
            };

            inline void SetColorRGB(glm::vec4 vec4color)
            {
                auto r = vec4color.r; // they are in range 0-255
                auto g = vec4color.g;
                auto b = vec4color.b;
                auto a = vec4color.a;

                color = ((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(r) << 0);
            };
        };

        enum class ShaderFragmentType {
            Solid,
            Image,
            SolidRound,
            ImageRound
        };

        typedef uint32_t BlendHandle;

        struct SubmitInfo
        {
            std::vector<Vertex>   vertices;
            std::vector<uint16_t> indices;
            glm::vec2             uiSize;
            glm::vec4             uiRadius;

            Rect clipRect;
            int  zIndex;

            const void        *image = NULL;
            ShaderFragmentType fragmentType;
            BlendHandle        alphablend;
        };

        enum class BlendFactor {
            BLEND_FACTOR_ZERO = 0,
            BLEND_FACTOR_ONE = 1,
            BLEND_FACTOR_SRC_COLOR = 2,
            BLEND_FACTOR_ONE_MINUS_SRC_COLOR = 3,
            BLEND_FACTOR_DST_COLOR = 4,
            BLEND_FACTOR_ONE_MINUS_DST_COLOR = 5,
            BLEND_FACTOR_SRC_ALPHA = 6,
            BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = 7,
            BLEND_FACTOR_DST_ALPHA = 8,
            BLEND_FACTOR_ONE_MINUS_DST_ALPHA = 9,
            BLEND_FACTOR_CONSTANT_COLOR = 10,
            BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 11,
            BLEND_FACTOR_CONSTANT_ALPHA = 12,
            BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 13,
            BLEND_FACTOR_SRC_ALPHA_SATURATE = 14,
            BLEND_FACTOR_SRC1_COLOR = 15,
            BLEND_FACTOR_ONE_MINUS_SRC1_COLOR = 16,
            BLEND_FACTOR_SRC1_ALPHA = 17,
            BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA = 18,
            BLEND_FACTOR_MAX_ENUM = 0x7FFFFFFF
        };

        enum class BlendOp {
            BLEND_OP_ADD = 0,
            BLEND_OP_SUBTRACT = 1,
            BLEND_OP_REVERSE_SUBTRACT = 2,
            BLEND_OP_MIN = 3,
            BLEND_OP_MAX = 4,
            BLEND_OP_MAX_ENUM = 0x7FFFFFFF
        };

        struct TextureBlendInfo
        {
            bool Enable;

            BlendFactor SrcColor;
            BlendFactor DstColor;
            BlendOp     ColorOp;

            BlendFactor SrcAlpha;
            BlendFactor DstAlpha;
            BlendOp     AlphaOp;
        };

        namespace DefaultBlend {
            const BlendHandle NONE = 0;  // no blending
            const BlendHandle BLEND = 1; // dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA)), dstA = srcA + (dstA * (1-srcA))
            const BlendHandle ADD = 2;   // dstRGB = (srcRGB * srcA) + dstRGB, dstA = dstA
            const BlendHandle MOD = 3;   // dstRGB = srcRGB * dstRGB, dstA = dstA
            const BlendHandle MUL = 4;   // dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA)), dstA = (srcA * dstA) + (dstA * (1-srcA))
        }                                // namespace DefaultBlend

        class Base
        {
        public:
            virtual ~Base() = default;

            virtual void Init() = 0;
            virtual void ReInit() = 0;
            virtual void Shutdown() = 0;

            virtual bool NeedReinit() = 0;

            virtual bool BeginFrame() = 0;
            virtual void EndFrame() = 0;

            virtual void ImGui_Init() = 0;
            virtual void ImGui_DeInit() = 0;
            virtual void ImGui_NewFrame() = 0;
            virtual void ImGui_EndFrame() = 0;

            virtual void Push(SubmitInfo &info) = 0;
            virtual void Push(std::vector<SubmitInfo> &infos) = 0;

            virtual void SetVSync(bool enabled) = 0;
            virtual void SetClearColor(glm::vec4 color) = 0;
            virtual void SetClearDepth(float depth) = 0;
            virtual void SetClearStencil(uint32_t stencil) = 0;

            virtual BlendHandle CreateBlendState(TextureBlendInfo blendInfo) = 0;
        };
    } // namespace Backends
} // namespace Graphics

#endif