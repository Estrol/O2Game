/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __VULKANDESCRIPTOR_H_
#define __VULKANDESCRIPTOR_H_

#include <Graphics/Utils/Rect.h>
#include <string.h>
#include <vulkan/vulkan.h>

namespace Graphics::Backends {
    struct VulkanDescriptor
    {
        uint32_t Id;

        VkDescriptorSet VkId;
        Rect            Size;
        int             Channels;

        VkImageView    ImageView;
        VkImage        Image;
        VkDeviceMemory ImageMemory;
        VkSampler      Sampler;

        VkBuffer       UploadBuffer;
        VkDeviceMemory UploadBufferMemory;

        VulkanDescriptor()
        {
            memset(this, 0, sizeof(VulkanDescriptor));
        }
    };
} // namespace Graphics::Backends

#endif