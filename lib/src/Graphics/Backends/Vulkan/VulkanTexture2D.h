/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __VULKANTEXTURE2D_H_
#define __VULKANTEXTURE2D_H_

#include <Graphics/GraphicsTexture2D.h>

namespace Graphics {
    namespace Backends {
        struct VulkanDescriptor;
    }

    class VKTexture2D : public Texture2D
    {
    public:
        VKTexture2D(TextureSamplerInfo samplerInfo);
        ~VKTexture2D() override;

        void Load(std::filesystem::path path) override;
        void Load(const char *buf, size_t size) override;
        void Load(const char *pixbuf, uint32_t width, uint32_t height) override;

        const void *GetId() override;
        const Rect  GetSize() override;

    private:
        Backends::VulkanDescriptor *Descriptor;
    };
} // namespace Graphics

#endif