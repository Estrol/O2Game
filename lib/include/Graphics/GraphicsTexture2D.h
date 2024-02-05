/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __GRAPHICSTEXTURE2D_H_
#define __GRAPHICSTEXTURE2D_H_

#include "./Utils/Rect.h"
#include <filesystem>

namespace Graphics {
    constexpr uint32_t kInvalidTexture = (uint32_t)-1;

    enum class TextureFilter {
        Nearest,
        Linear
    };

    enum class TextureAddressMode {
        Repeat,
        MirrorRepeat,
        ClampEdge,
        ClampBorder,
        MirrorClampEdge
    };

    enum class TextureCompareOP {
        COMPARE_OP_NEVER,
        COMPARE_OP_LESS,
        COMPARE_OP_EQUAL,
        COMPARE_OP_LESS_OR_EQUAL,
        COMPARE_OP_GREATER,
        COMPARE_OP_NOT_EQUAL,
        COMPARE_OP_GREATER_OR_EQUAL,
        COMPARE_OP_ALWAYS,
        COMPARE_OP_MAX_ENUM
    };

    constexpr float kMaxLOD = 1000.0f;

    struct TextureSamplerInfo
    {
        TextureFilter      FilterMag = TextureFilter::Linear;
        TextureFilter      FilterMin = TextureFilter::Linear;
        TextureAddressMode AddressModeU = TextureAddressMode::Repeat;
        TextureAddressMode AddressModeV = TextureAddressMode::Repeat;
        TextureAddressMode AddressModeW = TextureAddressMode::Repeat;

        bool AnisotropyEnable = false;
        bool CompareEnable = false;

        TextureCompareOP CompareOp = TextureCompareOP::COMPARE_OP_NEVER;

        float MipLodBias = 0;
        float MinLod = 0;
        float MaxLod = kMaxLOD;
        float MaxAnisotropy = 1.0f;
    };

    class Texture2D
    {
    public:
        Texture2D() = default;
        Texture2D(TextureSamplerInfo samplerInfo) : SamplerInfo(samplerInfo){};
        virtual ~Texture2D() = default;

        virtual void Load(std::filesystem::path path) = 0;
        virtual void Load(const unsigned char *buf, size_t size) = 0;
        virtual void Load(const unsigned char *pixbuf, uint32_t width, uint32_t height) = 0;

        virtual const void *GetId() = 0;
        virtual const Rect  GetSize() = 0;

    protected:
        std::filesystem::path Path;
        TextureSamplerInfo    SamplerInfo;
    };
} // namespace Graphics

#endif // __GRAPHICSTEXTURE2D_H_