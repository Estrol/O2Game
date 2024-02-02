/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "OpenGLTexture2D.h"
#include <Exceptions/EstException.h>
#include <Graphics/Renderer.h>
#include <Graphics/Utils/stb_image.h>
#include <Misc/Filesystem.h>

using namespace Graphics;
using namespace Exceptions;

GLTexture2D::GLTexture2D()
{
    memset(&Data, 0, sizeof(GlTexData));
    Data.Id = kInvalidTexture;
}

GLTexture2D::GLTexture2D(TextureSamplerInfo samplerInfo)
{
    memset(&Data, 0, sizeof(GlTexData));
    Data.Id = kInvalidTexture;

    SamplerInfo = samplerInfo;
}

GLTexture2D::~GLTexture2D()
{
}

void GLTexture2D::Load(std::filesystem::path path)
{
    if (Data.Id != kInvalidTexture) {
        throw Exceptions::EstException("Texture already loaded");
    }

    Path = path;

    auto data = Misc::Filesystem::ReadFile(path);

    Load((const char *)data.data(), data.size());
}

void GLTexture2D::Load(const char *buf, size_t size)
{
    if (Data.Id != kInvalidTexture) {
        throw Exceptions::EstException("Texture already loaded");
    }

    unsigned char *image_data = stbi_load_from_memory(
        (const unsigned char *)buf,
        (int)size,
        &Data.Size.Width,
        &Data.Size.Height,
        &Data.Channels,
        STBI_rgb_alpha);

    if (!image_data) {
        throw EstException("OpenGL Texture Error: %s", "image_data == null");
    }

    Data.Channels = 4; // always use RGBA
    Load((const char *)image_data, Data.Size.Width, Data.Size.Height);
    stbi_image_free(image_data);
}

void GLTexture2D::Load(const char *pixbuf, uint32_t width, uint32_t height)
{
    if (Data.Id != kInvalidTexture) {
        throw Exceptions::EstException("Texture already loaded");
    }

    auto renderer = Graphics::Renderer::Get();
    if (renderer->GetAPI() != Graphics::API::OpenGL) {
        throw EstException("Cannot load OpenGL texture from non-OpenGL renderer");
    }

    // get maximum texture size
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    if (width > (uint32_t)maxTextureSize || height > (uint32_t)maxTextureSize) {
        throw EstException("OpenGL Texture Error: %s", "Texture size too large");
    }

    glGenTextures(1, &Data.Id);
    glBindTexture(GL_TEXTURE_2D, Data.Id);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        throw EstException("OpenGL Texture Error: %d", err);
    }

    switch (SamplerInfo.FilterMag) {
        case TextureFilter::Nearest:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;

        case TextureFilter::Linear:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
    }

    switch (SamplerInfo.FilterMin) {
        case TextureFilter::Nearest:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            break;

        case TextureFilter::Linear:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;
    }

    switch (SamplerInfo.AddressModeU) {
        case TextureAddressMode::Repeat:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            break;

        case TextureAddressMode::ClampEdge:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            break;

        case TextureAddressMode::ClampBorder:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            break;

        case TextureAddressMode::MirrorRepeat:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            break;

        case TextureAddressMode::MirrorClampEdge:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE);
            break;
    }

    switch (SamplerInfo.AddressModeV) {
        case TextureAddressMode::Repeat:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;

        case TextureAddressMode::ClampEdge:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;

        case TextureAddressMode::ClampBorder:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            break;

        case TextureAddressMode::MirrorRepeat:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            break;

        case TextureAddressMode::MirrorClampEdge:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_EDGE);
            break;
    }

    switch (SamplerInfo.AddressModeW) {
        case TextureAddressMode::Repeat:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
            break;

        case TextureAddressMode::ClampEdge:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            break;

        case TextureAddressMode::ClampBorder:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
            break;

        case TextureAddressMode::MirrorRepeat:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
            break;

        case TextureAddressMode::MirrorClampEdge:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRROR_CLAMP_TO_EDGE);
            break;
    }

    err = glGetError();
    if (err != GL_NO_ERROR) {
        throw EstException("OpenGL Texture Error: %d", err);
    }

    if (SamplerInfo.AnisotropyEnable) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, SamplerInfo.MaxAnisotropy);
    } else {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
    }

    err = glGetError();
    if (err != GL_NO_ERROR) {
        throw EstException("OpenGL Texture Error: %d", err);
    }

    if (SamplerInfo.CompareEnable) {
        GLenum glCompareFunc;
        switch (SamplerInfo.CompareOp) {
            case TextureCompareOP::COMPARE_OP_ALWAYS:
                glCompareFunc = GL_ALWAYS;
                break;

            case TextureCompareOP::COMPARE_OP_NEVER:
                glCompareFunc = GL_NEVER;
                break;

            case TextureCompareOP::COMPARE_OP_LESS:
                glCompareFunc = GL_LESS;
                break;

            case TextureCompareOP::COMPARE_OP_EQUAL:
                glCompareFunc = GL_EQUAL;
                break;

            case TextureCompareOP::COMPARE_OP_LESS_OR_EQUAL:
                glCompareFunc = GL_LEQUAL;
                break;

            case TextureCompareOP::COMPARE_OP_GREATER:
                glCompareFunc = GL_GREATER;
                break;

            case TextureCompareOP::COMPARE_OP_NOT_EQUAL:
                glCompareFunc = GL_NOTEQUAL;
                break;

            case TextureCompareOP::COMPARE_OP_GREATER_OR_EQUAL:
                glCompareFunc = GL_GEQUAL;
                break;

            case TextureCompareOP::COMPARE_OP_MAX_ENUM:
                // Unsupported
                glCompareFunc = GL_ALWAYS;
                break;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, glCompareFunc);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }

    err = glGetError();
    if (err != GL_NO_ERROR) {
        throw EstException("OpenGL Texture Error: %d", err);
    }

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, SamplerInfo.MipLodBias);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, SamplerInfo.MinLod);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, SamplerInfo.MaxLod);

    err = glGetError();
    if (err != GL_NO_ERROR) {
        throw EstException("OpenGL Texture Error: %d", err);
    }

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixbuf);

    // check error
    err = glGetError();
    if (err != GL_NO_ERROR) {
        throw EstException("OpenGL Texture Error: %d", err);
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

const void *GLTexture2D::GetId()
{
    // Because OpenGL uses GLuint as Id not pointer, and the class template is using const void* as Id
    return reinterpret_cast<const void *>(static_cast<uintptr_t>(Data.Id));
}

const Rect GLTexture2D::GetSize()
{
    return Data.Size;
}