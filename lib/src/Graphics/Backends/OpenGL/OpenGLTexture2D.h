/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __OPENGLTEXTURE2D_H_
#define __OPENGLTEXTURE2D_H_

#include "./glad/gl.h"
#include <Graphics/GraphicsTexture2D.h>
#include <Graphics/Utils/Rect.h>

namespace Graphics {
    struct GlTexData
    {
        GLuint Id;

        Rect Size;
        int  Channels;
    };

    class GLTexture2D : public Texture2D
    {
    public:
        GLTexture2D(TextureSamplerInfo samplerInfo);
        GLTexture2D();
        ~GLTexture2D() override;

        void Load(std::filesystem::path path) override;
        void Load(const unsigned char *buf, size_t size) override;
        void Load(const unsigned char *pixbuf, uint32_t width, uint32_t height) override;

        const void *GetId() override;
        const Rect  GetSize() override;

    private:
        GlTexData Data;
    };
} // namespace Graphics

#endif