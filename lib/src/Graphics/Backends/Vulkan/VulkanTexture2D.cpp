/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "VulkanBackend.h"
#include "VulkanDescriptor.h"
#include "VulkanTexture2D.h"
#include "vkinit.h"
#include <Exceptions/EstException.h>
#include <Misc/Filesystem.h>
#include <fstream>

#include <Graphics/Renderer.h>
#include <Graphics/Utils/stb_image.h>

#include "../../ImguiBackends/imgui_impl_vulkan.h"

using namespace Graphics;
using namespace Exceptions;

VKTexture2D::VKTexture2D(TextureSamplerInfo samplerInfo)
{
    Descriptor = nullptr;
    SamplerInfo = samplerInfo;
}

VKTexture2D::~VKTexture2D()
{
    if (Descriptor) {
        auto renderer = Graphics::Renderer::Get();
        if (renderer->GetAPI() == Graphics::API::Vulkan) {
            auto vulkan = (Graphics::Backends::Vulkan *)renderer->GetBackend();
            vulkan->DestroyDescriptor(Descriptor);
        }
    }
}

void VKTexture2D::Load(std::filesystem::path path)
{
    if (Descriptor) {
        throw EstException("Cannot initialize texture twice");
    }

    auto data = Misc::Filesystem::ReadFile(path);

    Load((const unsigned char *)data.data(), data.size());
}

void VKTexture2D::Load(const unsigned char *buf, size_t size)
{
    if (Descriptor) {
        throw EstException("Cannot initialize texture twice");
    }

    auto renderer = Graphics::Renderer::Get();
    if (renderer->GetAPI() != Graphics::API::Vulkan) {
        throw EstException("Cannot load Vulkan texture from non-Vulkan renderer");
    }

    auto vulkan = (Graphics::Backends::Vulkan *)renderer->GetBackend();
    Descriptor = vulkan->CreateDescriptor();
    unsigned char *image_data = stbi_load_from_memory(
        (const unsigned char *)buf,
        (int)size,
        &Descriptor->Size.Width,
        &Descriptor->Size.Height,
        &Descriptor->Channels,
        STBI_rgb_alpha);

    if (!image_data) {
        throw EstException("Failed to load image");
    }

    Descriptor->Channels = 4; // always use RGBA

    Load((const unsigned char *)image_data, (uint32_t)Descriptor->Size.Width, (uint32_t)Descriptor->Size.Height);
    stbi_image_free(image_data);
}

void VKTexture2D::Load(const unsigned char *pixbuf, uint32_t width, uint32_t height)
{
    auto renderer = Graphics::Renderer::Get();
    if (renderer->GetAPI() != Graphics::API::Vulkan) {
        throw EstException("Cannot load Vulkan texture from non-Vulkan renderer");
    }

    auto vulkan = (Graphics::Backends::Vulkan *)renderer->GetBackend();
    auto vkobject = vulkan->GetVulkanObject();

    if (!Descriptor) {
        Descriptor = vulkan->CreateDescriptor();
        Descriptor->Channels = 4;
    }

    Descriptor->Size = {
        0, 0,
        (int)width, (int)height
    };

    size_t image_size = static_cast<size_t>(Descriptor->Size.Width) * static_cast<size_t>(Descriptor->Size.Height) * Descriptor->Channels;

    VkResult err = VK_SUCCESS;
    {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.extent.width = Descriptor->Size.Width;
        info.extent.height = Descriptor->Size.Height;
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        err = vkCreateImage(vkobject->vkbDevice.device, &info, nullptr, &Descriptor->Image);

        if (err != VK_SUCCESS) {
            throw EstException("Failed to create vulkan image");
        }

        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(vkobject->vkbDevice.device, Descriptor->Image, &req);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = req.size;
        alloc_info.memoryTypeIndex = vkinit::find_memory_type(
            vkobject->vkbDevice.physical_device,
            req.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        err = vkAllocateMemory(vkobject->vkbDevice.device, &alloc_info, nullptr, &Descriptor->ImageMemory);

        if (err != VK_SUCCESS) {
            throw EstException("Failed to allocate vulkan image memory");
        }

        err = vkBindImageMemory(vkobject->vkbDevice.device, Descriptor->Image, Descriptor->ImageMemory, 0);

        if (err != VK_SUCCESS) {
            throw EstException("Failed to bind vulkan image memory");
        }
    }

    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = Descriptor->Image;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = VK_FORMAT_R8G8B8A8_UNORM;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;

        err = vkCreateImageView(vkobject->vkbDevice.device, &info, nullptr, &Descriptor->ImageView);

        if (err != VK_SUCCESS) {
            throw EstException("Failed to create vulkan image view");
        }
    }

    {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        switch (SamplerInfo.FilterMag) {
            case TextureFilter::Nearest:
                samplerInfo.magFilter = VK_FILTER_NEAREST;
                break;

            case TextureFilter::Linear:
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                break;
        }

        switch (SamplerInfo.FilterMin) {
            case TextureFilter::Nearest:
                samplerInfo.minFilter = VK_FILTER_NEAREST;
                break;

            case TextureFilter::Linear:
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                break;
        }

        switch (SamplerInfo.AddressModeU) {
            case TextureAddressMode::Repeat:
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                break;

            case TextureAddressMode::ClampEdge:
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                break;

            case TextureAddressMode::ClampBorder:
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                break;

            case TextureAddressMode::MirrorRepeat:
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                break;

            case TextureAddressMode::MirrorClampEdge:
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                break;
        }

        switch (SamplerInfo.AddressModeV) {
            case TextureAddressMode::Repeat:
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                break;

            case TextureAddressMode::ClampEdge:
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                break;

            case TextureAddressMode::ClampBorder:
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                break;

            case TextureAddressMode::MirrorRepeat:
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                break;

            case TextureAddressMode::MirrorClampEdge:
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                break;
        }

        switch (SamplerInfo.AddressModeW) {
            case TextureAddressMode::Repeat:
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                break;

            case TextureAddressMode::ClampEdge:
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                break;

            case TextureAddressMode::ClampBorder:
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                break;

            case TextureAddressMode::MirrorRepeat:
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                break;

            case TextureAddressMode::MirrorClampEdge:
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                break;
        }

        samplerInfo.mipLodBias = SamplerInfo.MipLodBias;
        samplerInfo.anisotropyEnable = SamplerInfo.AnisotropyEnable;
        samplerInfo.maxAnisotropy = SamplerInfo.MaxAnisotropy;
        samplerInfo.compareEnable = SamplerInfo.CompareEnable;

        switch (SamplerInfo.CompareOp) {
            case TextureCompareOP::COMPARE_OP_ALWAYS:
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                break;

            case TextureCompareOP::COMPARE_OP_NEVER:
                samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
                break;

            case TextureCompareOP::COMPARE_OP_LESS:
                samplerInfo.compareOp = VK_COMPARE_OP_LESS;
                break;

            case TextureCompareOP::COMPARE_OP_EQUAL:
                samplerInfo.compareOp = VK_COMPARE_OP_EQUAL;
                break;

            case TextureCompareOP::COMPARE_OP_LESS_OR_EQUAL:
                samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                break;

            case TextureCompareOP::COMPARE_OP_GREATER:
                samplerInfo.compareOp = VK_COMPARE_OP_GREATER;
                break;

            case TextureCompareOP::COMPARE_OP_NOT_EQUAL:
                samplerInfo.compareOp = VK_COMPARE_OP_NOT_EQUAL;
                break;

            case TextureCompareOP::COMPARE_OP_GREATER_OR_EQUAL:
                samplerInfo.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
                break;

            case TextureCompareOP::COMPARE_OP_MAX_ENUM:
                samplerInfo.compareOp = VK_COMPARE_OP_MAX_ENUM;
                break;
        }

        samplerInfo.minLod = SamplerInfo.MinLod;
        samplerInfo.maxLod = SamplerInfo.MaxLod;

        err = vkCreateSampler(vkobject->vkbDevice.device, &samplerInfo, nullptr, &Descriptor->Sampler);
        if (err != VK_SUCCESS) {
            throw EstException("Failed to create vulkan sampler");
        }
    }

    Descriptor->VkId = ImGui_ImplVulkan_AddTexture(Descriptor->Sampler, Descriptor->ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    {
        VkDescriptorImageInfo desc_image[1] = {};
        desc_image[0].sampler = Descriptor->Sampler;
        desc_image[0].imageView = Descriptor->ImageView;
        desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write_desc[1] = {};
        write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc[0].dstSet = Descriptor->VkId;
        write_desc[0].descriptorCount = 1;
        write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc[0].pImageInfo = desc_image;

        vkUpdateDescriptorSets(vkobject->vkbDevice.device, 1, write_desc, 0, nullptr);
    }

    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = image_size;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        err = vkCreateBuffer(vkobject->vkbDevice.device, &buffer_info, nullptr, &Descriptor->UploadBuffer);

        if (err != VK_SUCCESS) {
            throw EstException("Failed to create vulkan upload buffer");
        }

        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(vkobject->vkbDevice.device, Descriptor->UploadBuffer, &req);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = req.size;
        alloc_info.memoryTypeIndex = vkinit::find_memory_type(
            vkobject->vkbDevice.physical_device,
            req.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        err = vkAllocateMemory(vkobject->vkbDevice.device, &alloc_info, nullptr, &Descriptor->UploadBufferMemory);

        if (err != VK_SUCCESS) {
            throw EstException("Failed to allocate vulkan upload buffer memory");
        }

        err = vkBindBufferMemory(vkobject->vkbDevice.device, Descriptor->UploadBuffer, Descriptor->UploadBufferMemory, 0);

        if (err != VK_SUCCESS) {
            throw EstException("Failed to bind vulkan upload buffer memory");
        }
    }

    {
        void *map = nullptr;

        err = vkMapMemory(vkobject->vkbDevice.device, Descriptor->UploadBufferMemory, 0, image_size, 0, &map);

        if (err != VK_SUCCESS) {
            throw EstException("Failed to map vulkan upload buffer memory");
        }

        memcpy(map, pixbuf, image_size);
        VkMappedMemoryRange range[1] = {};
        range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range[0].memory = Descriptor->UploadBufferMemory;
        range[0].size = image_size;

        err = vkFlushMappedMemoryRanges(vkobject->vkbDevice.device, 1, range);
        if (err != VK_SUCCESS) {
            throw EstException("Vulkan: Failed to flush mapped memory range");
        }

        vkUnmapMemory(vkobject->vkbDevice.device, Descriptor->UploadBufferMemory);
    }

    vulkan->ImmediateSubmit([=](VkCommandBuffer cmd) {
        VkImageMemoryBarrier copy_barrier[1] = {};
        copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier[0].image = Descriptor->Image;
        copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_barrier[0].subresourceRange.levelCount = 1;
        copy_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, copy_barrier);

        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = Descriptor->Size.Width;
        region.imageExtent.height = Descriptor->Size.Height;
        region.imageExtent.depth = 1;
        vkCmdCopyBufferToImage(cmd, Descriptor->UploadBuffer, Descriptor->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier use_barrier[1] = {};
        use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier[0].image = Descriptor->Image;
        use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        use_barrier[0].subresourceRange.levelCount = 1;
        use_barrier[0].subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, use_barrier);
    });
}

const void *VKTexture2D::GetId()
{
    return Descriptor->VkId;
}

const Rect VKTexture2D::GetSize()
{
    return Descriptor->Size;
}
