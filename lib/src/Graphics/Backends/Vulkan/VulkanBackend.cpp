/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "VulkanBackend.h"
#include <Exceptions/EstException.h>
#include <Graphics/NativeWindow.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <algorithm>
#include <array>
#include <vector>

#include "VulkanDescriptor.h"
#include "vkinit.h"

#include "../../Shaders/image.spv.h"
#include "../../Shaders/position.spv.h"
#include "../../Shaders/solid.spv.h"

#include "../../Shaders/image_round.spv.h"
#include "../../Shaders/position_round.spv.h"
#include "../../Shaders/solid_round.spv.h"

#include "../../ImguiBackends/imgui_impl_sdl2.h"
#include "../../ImguiBackends/imgui_impl_vulkan.h"
#include <Imgui/imgui_internal.h>

using namespace Graphics::Backends;

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
uint32_t           VkBlendOperatioId = 0;

struct PushConstant
{
    glm::vec4 ui_radius;
    glm::vec2 ui_size;

    glm::vec2 scale;
    glm::vec2 translate;
};

void Vulkan::Init()
{
    CreateInstance();
    InitSwapchain();
    InitMultiSampling();
    CreateRenderpass();
    InitFramebuffers();
    InitCommands();
    InitSyncStructures();
    InitDescriptors();
    InitShaders();
    InitPipeline();

    ImGui_Init();

    m_Initialized = true;
    m_FrameBegin = false;
}

void Vulkan::ReInit()
{
    vkDeviceWaitIdle(m_Vulkan.vkbDevice.device);

    m_PerFrameDeletionQueue.flush();
    m_SwapchainDeletionQueue.flush();

    try {
        bool result = InitSwapchain();
        if (!result)
            return;

        InitMultiSampling();
        InitFramebuffers();
        InitCommands();
        InitSyncStructures();
    } catch (const Exceptions::EstException &) {
        m_SwapchainReady = false;
    }
}

void Graphics::Backends::Vulkan::Shutdown()
{
    if (m_Initialized) {
        vkDeviceWaitIdle(m_Vulkan.vkbDevice.device);

        if (m_FrameBegin) {
            __debugbreak();
        }

        for (auto &descriptor : m_Descriptors) {
            DestroyDescriptor(descriptor.get(), false);
        }

        m_PerFrameDeletionQueue.flush();
        m_SwapchainDeletionQueue.flush();
        m_Descriptors.clear();
        m_DeletionQueue.flush();

        DestroyBuffers();
        ImGui_DeInit();

        vkb::destroy_swapchain(m_Swapchain.swapchain);
        vkDestroySurfaceKHR(m_Vulkan.vkbInstance.instance, m_Vulkan.surface, nullptr);

        vkb::destroy_device(m_Vulkan.vkbDevice);
        vkb::destroy_instance(m_Vulkan.vkbInstance);
    }
}

VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT) {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT) {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT) {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT) {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT) {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

/* Vulkan Create Instance */
void Vulkan::CreateInstance()
{
    memset(&m_Vulkan, 0, sizeof(m_Vulkan));

    if (volkInitialize() != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to initialize Vulkan");
    }

    vkb::InstanceBuilder instance_builder;
    auto                 instance_builder_return = instance_builder
                                       .set_app_name("EstEngineLibrary")
                                       .set_engine_name("EstEngine")
                                       .request_validation_layers(true)
                                       .require_api_version(1, 1, 0)
                                       //.use_default_debug_messenger()
                                       .build();

    if (!instance_builder_return) {
        throw Exceptions::EstException("Failed to create Vulkan instance");
    }

    vkb::Instance vkb_instance = instance_builder_return.value();

    volkLoadInstance(vkb_instance.instance);

    if (!SDL_Vulkan_CreateSurface(
            Graphics::NativeWindow::Get()->GetWindow(),
            vkb_instance.instance,
            &m_Vulkan.surface)) {
        throw Exceptions::EstException("Failed to create Vulkan surface");
    }

    vkb::PhysicalDeviceSelector selector{ vkb_instance };
    vkb::PhysicalDevice         physical_device = selector
                                              .set_minimum_version(1, 1)
                                              .set_surface(m_Vulkan.surface)
                                              .select()
                                              .value();

    vkb::DeviceBuilder device_builder{ physical_device };
    vkb::Device        vkb_device = device_builder.build().value();

    volkLoadDevice(vkb_device.device);

    auto queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    auto queueFamily = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
    auto maxSampleCount = GetMaxUsableSampleCount(vkb_device.physical_device);

    m_Vulkan.graphicsQueue = queue;
    m_Vulkan.graphicsQueueFamily = queueFamily;
    m_Vulkan.vkbInstance = vkb_instance;
    m_Vulkan.vkbDevice = vkb_device;
    m_Vulkan.MSAASampleCount = maxSampleCount;
}

bool Vulkan::InitSwapchain()
{
    auto rect = Graphics::NativeWindow::Get()->GetWindowSize();

    VkPresentModeKHR preset = VK_PRESENT_MODE_FIFO_KHR;
    if (!m_VSync) {
        preset = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    vkb::SwapchainBuilder builder{ m_Vulkan.vkbDevice };
    builder.set_desired_format({ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_old_swapchain(m_Swapchain.swapchain)
        .set_desired_extent((uint32_t)rect.Width, (uint32_t)rect.Height)
        .set_desired_present_mode(preset);

    auto resultbuild = builder.build();
    if (!resultbuild) {
        if (resultbuild.error() == vkb::SwapchainError::invalid_window_size) {
            vkb::destroy_swapchain(m_Swapchain.swapchain);
        }

        m_Swapchain.swapchain.swapchain = VK_NULL_HANDLE;
        return false;
    }

    vkb::destroy_swapchain(m_Swapchain.swapchain);
    m_Swapchain.swapchain = resultbuild.value();

    auto vkimages = m_Swapchain.swapchain.get_images();
    if (!vkimages) {
        throw Exceptions::EstException("Failed to get swapchain images");
    }

    auto vkbviews = m_Swapchain.swapchain.get_image_views();
    if (!vkbviews) {
        throw Exceptions::EstException("Failed to get swapchain image views");
    }

    m_Swapchain.imageViews = vkbviews.value();
    m_Swapchain.images = vkimages.value();

    m_Vulkan.depthFormat = VK_FORMAT_D32_SFLOAT;
    m_Vulkan.swapchainFormat = m_Swapchain.swapchain.image_format;

    auto swapchainExtent = m_Swapchain.swapchain.extent;

    VkExtent3D depthImageExtent = {
        (uint32_t)swapchainExtent.width,
        (uint32_t)swapchainExtent.height,
        1
    };

    VkImageCreateInfo dimg_info = vkinit::image_create_info(m_Vulkan.depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);
    auto              result = vkCreateImage(m_Vulkan.vkbDevice.device, &dimg_info, nullptr, &m_Swapchain.depthImage);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to allocate depth image memory");
    }

    // allocate memory
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Vulkan.vkbDevice.device, m_Swapchain.depthImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vkinit::find_memory_type(m_Vulkan.vkbDevice.physical_device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        result = vkAllocateMemory(m_Vulkan.vkbDevice.device, &allocInfo, nullptr, &m_Swapchain.depthImageMemory);
        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to allocate image memory");
        }

        vkBindImageMemory(m_Vulkan.vkbDevice.device, m_Swapchain.depthImage, m_Swapchain.depthImageMemory, 0);
    }

    VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(m_Vulkan.depthFormat, m_Swapchain.depthImage, VK_IMAGE_ASPECT_DEPTH_BIT);
    result = vkCreateImageView(m_Vulkan.vkbDevice.device, &dview_info, nullptr, &m_Swapchain.depthImageView);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to allocate depth image memory");
    }

    m_SwapchainDeletionQueue.push_function([=] {
        vkDestroyImageView(m_Vulkan.vkbDevice.device, m_Swapchain.depthImageView, nullptr);
        vkDestroyImage(m_Vulkan.vkbDevice.device, m_Swapchain.depthImage, nullptr);
        vkFreeMemory(m_Vulkan.vkbDevice.device, m_Swapchain.depthImageMemory, nullptr);

        m_Swapchain.depthImageView = VK_NULL_HANDLE;
        m_Swapchain.depthImage = VK_NULL_HANDLE;
        m_Swapchain.depthImageMemory = VK_NULL_HANDLE;
    });

    m_SwapchainReady = true;
    return true;
}

void Vulkan::CreateRenderpass()
{
    std::array<VkAttachmentDescription, 3> attachments = {};

    attachments[0].format = m_Vulkan.swapchainFormat;
    attachments[0].samples = m_Vulkan.MSAASampleCount;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[1].format = m_Vulkan.swapchainFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[2].format = m_Vulkan.depthFormat;
    attachments[2].samples = m_Vulkan.MSAASampleCount;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {};
    depth_attachment_ref.attachment = 2;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_resolve_ref = {};
    color_resolve_ref.attachment = 1;
    color_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pResolveAttachments = &color_resolve_ref;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    std::array<VkSubpassDependency, 2> dependencies = {};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    render_pass_info.pDependencies = dependencies.data();

    auto result = vkCreateRenderPass(m_Vulkan.vkbDevice.device, &render_pass_info, nullptr, &m_Swapchain.renderpass);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create render pass");
    }

    m_DeletionQueue.push_function([=]() { vkDestroyRenderPass(m_Vulkan.vkbDevice.device, m_Swapchain.renderpass, nullptr); });
}

void Vulkan::InitFramebuffers()
{
    auto                    rect = Graphics::NativeWindow::Get()->GetWindowSize();
    VkExtent2D              _windowExtent = { (uint32_t)rect.Width, (uint32_t)rect.Height };
    VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(m_Swapchain.renderpass, _windowExtent);

    const uint32_t swapchain_imagecount = (uint32_t)m_Swapchain.images.size();
    m_Swapchain.framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

    for (uint32_t i = 0; i < swapchain_imagecount; i++) {
        VkImageView attachments[3];
        // attachments[0] = m_Swapchain.imageViews[i];
        // attachments[1] = m_Swapchain.depthImageView;

        attachments[0] = m_MultiSamplingTarget.color.view;
        attachments[1] = m_Swapchain.imageViews[i];
        attachments[2] = m_MultiSamplingTarget.depth.view;

        fb_info.pAttachments = attachments;
        fb_info.attachmentCount = 3;

        auto result = vkCreateFramebuffer(m_Vulkan.vkbDevice.device, &fb_info, nullptr, &m_Swapchain.framebuffers[i]);
        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to create framebuffer");
        }

        m_SwapchainDeletionQueue.push_function([=]() {
            vkDestroyFramebuffer(m_Vulkan.vkbDevice.device, m_Swapchain.framebuffers[i], nullptr);
            vkDestroyImageView(m_Vulkan.vkbDevice.device, m_Swapchain.imageViews[i], nullptr);

            m_Swapchain.framebuffers[i] = VK_NULL_HANDLE;
            m_Swapchain.imageViews[i] = VK_NULL_HANDLE;
        });
    }
}

void Vulkan::InitCommands()
{
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(m_Vulkan.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    m_Swapchain.frames.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto result = vkCreateCommandPool(m_Vulkan.vkbDevice.device, &commandPoolInfo, nullptr, &m_Swapchain.frames[i].commandPool);

        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to create command pool");
        }

        VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(m_Swapchain.frames[i].commandPool, 1);

        result = vkAllocateCommandBuffers(m_Vulkan.vkbDevice.device, &cmdAllocInfo, &m_Swapchain.frames[i].commandBuffer);

        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to allocate command buffer");
        }

        m_Swapchain.frames[i].isValid = true;
        m_SwapchainDeletionQueue.push_function([=]() {
            vkDestroyCommandPool(m_Vulkan.vkbDevice.device, m_Swapchain.frames[i].commandPool, nullptr);
            m_Swapchain.frames[i].commandPool = VK_NULL_HANDLE; });
    }

    VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(m_Vulkan.graphicsQueueFamily);
    auto                    result = vkCreateCommandPool(m_Vulkan.vkbDevice.device, &uploadCommandPoolInfo, nullptr, &m_Swapchain.uploadContext.commandPool);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create upload command pool");
    }

    m_SwapchainDeletionQueue.push_function([=]() {
        vkDestroyCommandPool(m_Vulkan.vkbDevice.device, m_Swapchain.uploadContext.commandPool, nullptr);
    });

    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(m_Swapchain.uploadContext.commandPool, 1);

    result = vkAllocateCommandBuffers(m_Vulkan.vkbDevice.device, &cmdAllocInfo, &m_Swapchain.uploadContext.commandBuffer);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to allocate upload command buffer");
    }

    constexpr uint32_t MAX_VERTEX_OBJECTS = 50000;
    constexpr uint32_t MAX_VERTEX_BUFFER_SIZE = sizeof(Vertex) * MAX_VERTEX_OBJECTS;
    constexpr uint32_t MAX_INDEX_BUFFER_SIZE = sizeof(uint32_t) * MAX_VERTEX_OBJECTS;

    memset(&m_Swapchain.vertexBuffer, 0, sizeof(m_Swapchain.vertexBuffer));
    memset(&m_Swapchain.indexBuffer, 0, sizeof(m_Swapchain.indexBuffer));

    ResizeBuffer(MAX_VERTEX_BUFFER_SIZE, MAX_INDEX_BUFFER_SIZE);

    m_Swapchain.maxVertexBufferSize = MAX_VERTEX_BUFFER_SIZE;
    m_Swapchain.maxIndexBufferSize = MAX_INDEX_BUFFER_SIZE;
}

void Vulkan::InitSyncStructures()
{
    VkFenceCreateInfo     fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto result = vkCreateFence(m_Vulkan.vkbDevice.device, &fenceCreateInfo, nullptr, &m_Swapchain.frames[i].renderFence);

        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to create fence");
        }

        m_SwapchainDeletionQueue.push_function([=]() { vkDestroyFence(m_Vulkan.vkbDevice.device, m_Swapchain.frames[i].renderFence, nullptr); });

        result = vkCreateSemaphore(m_Vulkan.vkbDevice.device, &semaphoreCreateInfo, nullptr, &m_Swapchain.frames[i].presentSemaphore);

        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to create preset fence");
        }

        result = vkCreateSemaphore(m_Vulkan.vkbDevice.device, &semaphoreCreateInfo, nullptr, &m_Swapchain.frames[i].renderSemaphore);

        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to create render fence");
        }

        m_SwapchainDeletionQueue.push_function([=]() {
            vkDestroySemaphore(m_Vulkan.vkbDevice.device, m_Swapchain.frames[i].presentSemaphore, nullptr);
            vkDestroySemaphore(m_Vulkan.vkbDevice.device, m_Swapchain.frames[i].renderSemaphore, nullptr); });
    }

    auto result = vkCreateFence(m_Vulkan.vkbDevice.device, &fenceCreateInfo, nullptr, &m_Swapchain.uploadContext.renderFence);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create upload fence");
    }

    m_SwapchainDeletionQueue.push_function([=]() { vkDestroyFence(m_Vulkan.vkbDevice.device, m_Swapchain.uploadContext.renderFence, nullptr); });
}

void Vulkan::InitDescriptors()
{
    std::vector<VkDescriptorPoolSize> sizes = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 10;
    poolInfo.poolSizeCount = (uint32_t)sizes.size();
    poolInfo.pPoolSizes = sizes.data();

    auto result = vkCreateDescriptorPool(m_Vulkan.vkbDevice.device, &poolInfo, nullptr, &m_Vulkan.descriptorPool);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create descriptor pool");
    }

    VkDescriptorSetLayoutBinding binding[1] = {};
    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding[0].descriptorCount = 1;
    binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = binding;

    result = vkCreateDescriptorSetLayout(m_Vulkan.vkbDevice.device, &info, nullptr, &m_Vulkan.descriptorSetLayout);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create descriptor set layout");
    }

    m_DeletionQueue.push_function([=] {
        vkDestroyDescriptorSetLayout(m_Vulkan.vkbDevice.device, m_Vulkan.descriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(m_Vulkan.vkbDevice.device, m_Vulkan.descriptorPool, nullptr);
    });
}

void Vulkan::InitShaders()
{
    const uint32_t *vertShaderCode = __glsl_position;
    const uint32_t *solidFragShaderCode = __glsl_solid;
    const uint32_t *imageFragShaderCode = __glsl_image;

    size_t vertShaderSize = sizeof(__glsl_position);
    size_t solidFragShaderSize = sizeof(__glsl_solid);
    size_t imageFragShaderSize = sizeof(__glsl_image);

    VkShaderModuleCreateInfo vertShaderInfo = {};
    vertShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderInfo.codeSize = vertShaderSize;
    vertShaderInfo.pCode = vertShaderCode;

    VkShaderModuleCreateInfo solidFragShaderInfo = {};
    solidFragShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    solidFragShaderInfo.codeSize = solidFragShaderSize;
    solidFragShaderInfo.pCode = solidFragShaderCode;

    VkShaderModuleCreateInfo imageFragShaderInfo = {};
    imageFragShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    imageFragShaderInfo.codeSize = imageFragShaderSize;
    imageFragShaderInfo.pCode = imageFragShaderCode;

    auto result = vkCreateShaderModule(m_Vulkan.vkbDevice.device, &vertShaderInfo, nullptr, &m_Vulkan.vertShaderModule);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create vertex shader module");
    }

    result = vkCreateShaderModule(m_Vulkan.vkbDevice.device, &solidFragShaderInfo, nullptr, &m_Vulkan.solidFragShaderModule);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create solid fragment shader module");
    }

    result = vkCreateShaderModule(m_Vulkan.vkbDevice.device, &imageFragShaderInfo, nullptr, &m_Vulkan.imageFragShaderModule);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create image fragment shader module");
    }

    m_DeletionQueue.push_function([=]() {
        vkDestroyShaderModule(m_Vulkan.vkbDevice.device, m_Vulkan.vertShaderModule, nullptr);
        vkDestroyShaderModule(m_Vulkan.vkbDevice.device, m_Vulkan.solidFragShaderModule, nullptr);
        vkDestroyShaderModule(m_Vulkan.vkbDevice.device, m_Vulkan.imageFragShaderModule, nullptr); });
}

void Vulkan::InitPipeline()
{
    // NONE, no blending
    // BLEND, dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA)), dstA = srcA + (dstA * (1-srcA))
    // ADD, dstRGB = (srcRGB * srcA) + dstRGB, dstA = dstA
    // MOD, dstRGB = srcRGB * dstRGB, dstA = dstA
    // MUL dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA)), dstA = (srcA * dstA) + (dstA * (1-srcA))

    TextureBlendInfo blendNone = {
        true,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendOp::BLEND_OP_ADD
    };

    TextureBlendInfo blendBlend = {
        true,
        BlendFactor::BLEND_FACTOR_SRC_ALPHA,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_SRC_ALPHA,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD
    };

    TextureBlendInfo blendAdd = {
        true,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendOp::BLEND_OP_ADD
    };

    TextureBlendInfo blendMod = {
        true,
        BlendFactor::BLEND_FACTOR_DST_COLOR,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_ONE,
        BlendFactor::BLEND_FACTOR_ZERO,
        BlendOp::BLEND_OP_ADD
    };

    TextureBlendInfo blendMul = {
        true,
        BlendFactor::BLEND_FACTOR_SRC_COLOR,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD,
        BlendFactor::BLEND_FACTOR_SRC_ALPHA,
        BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        BlendOp::BLEND_OP_ADD
    };

    CreateBlendState(blendNone);
    CreateBlendState(blendBlend);
    CreateBlendState(blendAdd);
    CreateBlendState(blendMod);
    CreateBlendState(blendMul);
}

void Vulkan::InitMultiSampling()
{
    auto       window = Graphics::NativeWindow::Get()->GetWindowSize();
    VkExtent3D extent = { (uint32_t)window.Width, (uint32_t)window.Height, (uint32_t)0 };
    extent.depth = 1;

    auto info = vkinit::image_create_info(
        m_Vulkan.swapchainFormat,
        m_Vulkan.MSAASampleCount,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        extent);

    auto result = vkCreateImage(m_Vulkan.vkbDevice.device, &info, nullptr, &m_MultiSamplingTarget.color.image);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create MSAA multi sampling color image");
    }

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.color.image, &memReqs);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = vkinit::find_memory_type(
        m_Vulkan.vkbDevice.physical_device,
        memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);

    if (allocInfo.memoryTypeIndex == (uint32_t)-1) {
        allocInfo.memoryTypeIndex = vkinit::find_memory_type(
            m_Vulkan.vkbDevice.physical_device,
            memReqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    result = vkAllocateMemory(m_Vulkan.vkbDevice.device, &allocInfo, nullptr, &m_MultiSamplingTarget.color.memory);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create MSAA multi sampling color image #2");
    }

    vkBindImageMemory(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.color.image, m_MultiSamplingTarget.color.memory, 0);

    auto viewInfo = vkinit::imageview_create_info(
        m_Vulkan.swapchainFormat,
        m_MultiSamplingTarget.color.image,
        VK_IMAGE_ASPECT_COLOR_BIT);

    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

    result = vkCreateImageView(m_Vulkan.vkbDevice.device, &viewInfo, nullptr, &m_MultiSamplingTarget.color.view);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create MSAA multi sampling color image #3");
    }

    // Depth target
    info.format = m_Vulkan.depthFormat;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    result = vkCreateImage(m_Vulkan.vkbDevice.device, &info, nullptr, &m_MultiSamplingTarget.depth.image);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create MSAA multi sampling depth image");
    }

    vkGetImageMemoryRequirements(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.depth.image, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = vkinit::find_memory_type(
        m_Vulkan.vkbDevice.physical_device,
        memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);

    if (allocInfo.memoryTypeIndex == (uint32_t)-1) {
        allocInfo.memoryTypeIndex = vkinit::find_memory_type(
            m_Vulkan.vkbDevice.physical_device,
            memReqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    result = vkAllocateMemory(m_Vulkan.vkbDevice.device, &allocInfo, nullptr, &m_MultiSamplingTarget.depth.memory);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create MSAA multi sampling depth image #2");
    }

    vkBindImageMemory(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.depth.image, m_MultiSamplingTarget.depth.memory, 0);

    viewInfo = vkinit::imageview_create_info(
        m_Vulkan.depthFormat,
        m_MultiSamplingTarget.depth.image,
        VK_IMAGE_ASPECT_DEPTH_BIT);

    if (m_Vulkan.depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

    result = vkCreateImageView(m_Vulkan.vkbDevice.device, &viewInfo, nullptr, &m_MultiSamplingTarget.depth.view);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to create MSAA multi sampling depth image #3");
    }

    m_SwapchainDeletionQueue.push_function([=] {
        vkDestroyImageView(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.color.view, nullptr);
        vkDestroyImage(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.color.image, nullptr);
        vkFreeMemory(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.color.memory, nullptr);

        vkDestroyImageView(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.depth.view, nullptr);
        vkDestroyImage(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.depth.image, nullptr);
        vkFreeMemory(m_Vulkan.vkbDevice.device, m_MultiSamplingTarget.depth.memory, nullptr);
    });
}

void Vulkan::ImmediateSubmit(std::function<void(VkCommandBuffer)> &&function)
{
    vkWaitForFences(m_Vulkan.vkbDevice.device, 1, &m_Swapchain.uploadContext.renderFence, true, 9999999999);
    vkResetFences(m_Vulkan.vkbDevice.device, 1, &m_Swapchain.uploadContext.renderFence);

    vkResetCommandPool(m_Vulkan.vkbDevice.device, m_Swapchain.uploadContext.commandPool, 0);

    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    auto                     result = vkBeginCommandBuffer(m_Swapchain.uploadContext.commandBuffer, &cmdBeginInfo);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to begin command buffer");
    }

    function(m_Swapchain.uploadContext.commandBuffer);

    result = vkEndCommandBuffer(m_Swapchain.uploadContext.commandBuffer);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to end command buffer");
    }

    VkSubmitInfo submit = vkinit::submit_info(&m_Swapchain.uploadContext.commandBuffer);

    result = vkQueueSubmit(m_Vulkan.graphicsQueue, 1, &submit, m_Swapchain.uploadContext.renderFence);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to submit queue");
    }
}

VulkanFrame &Vulkan::GetCurrentFrame()
{
    return m_Swapchain.frames[m_CurrentFrame % MAX_FRAMES_IN_FLIGHT];
}

VulkanFrame &Vulkan::GetLastFrame()
{
    return m_Swapchain.frames[(m_CurrentFrame - 1) % MAX_FRAMES_IN_FLIGHT];
}

bool Vulkan::BeginFrame()
{
    if (!m_SwapchainReady) {
        return false;
    }

    auto result = vkDeviceWaitIdle(m_Vulkan.vkbDevice.device);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to wait for device idle");
    }

    auto &frame = GetCurrentFrame();

    result = vkWaitForFences(m_Vulkan.vkbDevice.device, 1, &frame.renderFence, true, 9999999999);
    if (result == VK_TIMEOUT) {
        return false;
    } else if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to wait for fence");
    }

    result = vkResetFences(m_Vulkan.vkbDevice.device, 1, &frame.renderFence);
    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to reset fence");
    }

    result = vkAcquireNextImageKHR(m_Vulkan.vkbDevice.device, m_Swapchain.swapchain, 9999999999, frame.presentSemaphore, VK_NULL_HANDLE, &m_Swapchain.swapchainIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_SwapchainReady = false;
        return false;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw Exceptions::EstException("Failed to acquire next image");
    }

    result = vkResetFences(m_Vulkan.vkbDevice.device, 1, &frame.renderFence);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to reset fence");
    }

    result = vkResetCommandPool(m_Vulkan.vkbDevice.device, frame.commandPool, 0);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to reset command pool");
    }

    m_PerFrameDeletionQueue.flush();

    if (!frame.isValid) {
        return false;
    }

    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    result = vkBeginCommandBuffer(frame.commandBuffer, &cmdBeginInfo);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to begin command buffer");
    }

    VkClearValue clearValue;
    clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkClearValue clearValue2;
    clearValue2.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    auto       rect = Graphics::NativeWindow::Get()->GetWindowSize();
    VkExtent2D _windowExtent = {
        (uint32_t)rect.Width,
        (uint32_t)rect.Height
    };

    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;

    VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(m_Swapchain.renderpass, _windowExtent, m_Swapchain.framebuffers[m_Swapchain.swapchainIndex]);

    VkClearValue clearValues[] = { clearValue, clearValue2, depthClear };
    rpInfo.pClearValues = &clearValues[0];
    rpInfo.clearValueCount = 3;

    vkCmdBeginRenderPass(frame.commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_FrameBegin = true;

    return true;
}

void Vulkan::EndFrame()
{
    if (!m_SwapchainReady) {
        throw Exceptions::EstException("Attempt to end-frame on swap chain not ready");
    }

    auto &frame = GetCurrentFrame();

    FlushQueue();
    if (m_HasImgui) {
        m_HasImgui = false;

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frame.commandBuffer);
    }

    vkCmdEndRenderPass(frame.commandBuffer);

    auto result = vkEndCommandBuffer(frame.commandBuffer);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to end command buffer");
    }

    VkSubmitInfo         submit = vkinit::submit_info(&frame.commandBuffer);
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &frame.presentSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &frame.renderSemaphore;

    result = vkQueueSubmit(m_Vulkan.graphicsQueue, 1, &submit, frame.renderFence);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to submit queue");
    }

    VkPresentInfoKHR presentInfo = vkinit::present_info();
    presentInfo.pSwapchains = &m_Swapchain.swapchain.swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &frame.renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &m_Swapchain.swapchainIndex;

    result = vkQueuePresentKHR(m_Vulkan.graphicsQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        m_SwapchainReady = false;
    } else {
        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to submit swap buffer");
        }
    }

    m_CurrentFrame++;
    m_FrameBegin = false;
}

bool Vulkan::NeedReinit()
{
    return !m_SwapchainReady;
}

void Vulkan::Push(SubmitInfo &info)
{
    if (info.vertices.size() == 0) {
        return;
    }

    uint32_t maxSize = (m_DrawData.vertexSize + static_cast<uint32_t>(info.vertices.size())) * sizeof(info.vertices[0]);
    uint32_t inMaxSize = (m_DrawData.indiceSize + static_cast<uint32_t>(info.indices.size())) * sizeof(info.indices[0]);

    if (maxSize >= m_Swapchain.maxVertexBufferSize || inMaxSize >= m_Swapchain.maxIndexBufferSize) {
        ResizeBuffer(maxSize, inMaxSize);
    }

    for (auto &it : info.vertices) {
        m_DrawData.vertexPtr[m_DrawData.vertexSize++] = it;
    }

    for (auto &it : info.indices) {
        m_DrawData.indexPtr[m_DrawData.indiceSize++] = it;
    }

    VulkanDrawItem item = {};
    item.blend = info.alphablend;
    item.count = (uint32_t)info.indices.size();
    item.instanceCount = 1;
    item.type = info.fragmentType;
    item.uiRadius = info.uiRadius;
    item.image = info.image;
    item.uiSize = info.uiSize;
    item.clipRect = info.clipRect;

    submitInfos.push_back(item);
}

void Vulkan::Push(std::vector<SubmitInfo> &infos)
{
    if (infos.size() == 0) {
        return;
    }

    size_t size = infos.size();

    uint32_t maxSize = (m_DrawData.vertexSize + static_cast<uint32_t>(infos[0].vertices.size() * size)) * sizeof(infos[0].vertices[0]);
    uint32_t inMaxSize = (m_DrawData.indiceSize + static_cast<uint32_t>(infos[0].indices.size() * size)) * sizeof(infos[0].indices[0]);

    if (maxSize >= m_Swapchain.maxVertexBufferSize || inMaxSize >= m_Swapchain.maxIndexBufferSize) {
        ResizeBuffer(maxSize, inMaxSize);
    }

    uint16_t index_increment = 0;
    for (auto &info : infos) {
        for (auto &it : info.vertices) {
            m_DrawData.vertexPtr[m_DrawData.vertexSize++] = it;
        }

        for (auto &it : info.indices) {
            m_DrawData.indexPtr[m_DrawData.indiceSize++] = index_increment + it;
        }

        index_increment += (uint16_t)info.indices.size();
    }

    VulkanDrawItem item = {};
    item.blend = infos[0].alphablend;
    item.count = (uint32_t)infos[0].indices.size();
    item.instanceCount = (uint32_t)size;
    item.type = infos[0].fragmentType;
    item.uiRadius = infos[0].uiRadius;
    item.image = infos[0].image;
    item.uiSize = infos[0].uiSize;
    item.clipRect = infos[0].clipRect;

    submitInfos.push_back(item);
}

void Vulkan::FlushQueue()
{
    if (submitInfos.size() <= 0) {
        return;
    }

    if (!m_SwapchainReady) {
        submitInfos.clear();
        return;
    }

    auto &frame = GetCurrentFrame();
    if (!frame.isValid) {
        submitInfos.clear();
        return;
    }

    auto rect = Graphics::NativeWindow::Get()->GetWindowSize();

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)(rect.Width <= 0 ? 1 : rect.Width);
    viewport.height = (float)(rect.Height <= 0 ? 1 : rect.Height);
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(frame.commandBuffer, 0, 1, &viewport);

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, &m_Swapchain.vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(frame.commandBuffer, m_Swapchain.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    int currentVertIndex = 0; // vertex
    int currentIndiIndex = 0; // indicies

    PushConstant pc = {};

    pc.scale = glm::vec2(2.0f / rect.Width, 2.0f / rect.Height);
    pc.translate = glm::vec2(-1.0f, -1.0f);

    for (auto &info : submitInfos) {
        auto &blendinfo = m_BlendStates[info.blend];
        auto &pipeline = m_Swapchain.pipelineLayout;
        auto &graphics = blendinfo.pipelines[info.type];

        pc.ui_size = info.uiSize;
        pc.ui_radius = info.uiRadius;

        vkCmdPushConstants(frame.commandBuffer, pipeline, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &pc);

        VkDescriptorSet image = (VkDescriptorSet)(info.image != 0 ? (void *)info.image : VK_NULL_HANDLE);
        uint32_t        imageCount = 1;
        if (image == NULL) {
            imageCount = 0;
        }

        vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline, 0, 1, &image, 0, nullptr);
        vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics);

        VkRect2D clip = {};
        clip.offset = {
            std::max(0, info.clipRect.X),
            std::max(0, info.clipRect.Y)
        };
        clip.extent = {
            static_cast<uint32_t>(std::max(0, info.clipRect.Width - info.clipRect.X)),
            static_cast<uint32_t>(std::max(0, info.clipRect.Height - info.clipRect.Y))
        };

        vkCmdSetScissor(frame.commandBuffer, 0, 1, &clip);

        vkCmdDrawIndexed(
            frame.commandBuffer,
            info.count * info.instanceCount,
            1,
            currentIndiIndex,
            currentVertIndex,
            0);

        currentVertIndex += (int)info.count * info.instanceCount;
        currentIndiIndex += (int)info.count * info.instanceCount;

        if (info.instanceCount > 1) {
            //__debugbreak();
        }
    }

    submitInfos.clear();
    m_DrawData.Reset();
}

void Vulkan::ResizeBuffer(VkDeviceSize vertices, VkDeviceSize indicies)
{
    auto result = VK_SUCCESS;

    if (vertices > m_Swapchain.maxVertexBufferSize) {
        m_Swapchain.maxVertexBufferSize = (uint32_t)vertices;
    }

    if (indicies > m_Swapchain.maxIndexBufferSize) {
        m_Swapchain.maxIndexBufferSize = (uint32_t)indicies;
    }

    std::vector<Vertex>   backupVertices;
    std::vector<uint16_t> backupIndex;

    {
        backupVertices.resize(m_DrawData.vertexSize);
        backupIndex.resize(m_DrawData.indiceSize);

        memcpy(backupVertices.data(), m_DrawData.vertexPtr, m_DrawData.vertexSize * sizeof(Vertex));
        memcpy(backupIndex.data(), m_DrawData.indexPtr, m_DrawData.indiceSize * sizeof(uint16_t));
    }

    DestroyBuffers();

    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = vertices;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        result = vkCreateBuffer(m_Vulkan.vkbDevice.device, &bufferInfo, nullptr, &m_Swapchain.vertexBuffer.buffer);
        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to create vertex buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Vulkan.vkbDevice.device, m_Swapchain.vertexBuffer.buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vkinit::find_memory_type(
            m_Vulkan.vkbDevice.physical_device,
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        result = vkAllocateMemory(m_Vulkan.vkbDevice.device, &allocInfo, nullptr, &m_Swapchain.vertexBuffer.memory);

        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to allocate vertex buffer memory");
        }

        vkBindBufferMemory(m_Vulkan.vkbDevice.device, m_Swapchain.vertexBuffer.buffer, m_Swapchain.vertexBuffer.memory, 0);

        vkMapMemory(m_Vulkan.vkbDevice.device, m_Swapchain.vertexBuffer.memory, 0, vertices, 0, reinterpret_cast<void **>(&m_DrawData.vertexPtr));
    }

    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = indicies;
        bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        result = vkCreateBuffer(m_Vulkan.vkbDevice.device, &bufferInfo, nullptr, &m_Swapchain.indexBuffer.buffer);
        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to create index buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Vulkan.vkbDevice.device, m_Swapchain.indexBuffer.buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vkinit::find_memory_type(
            m_Vulkan.vkbDevice.physical_device,
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        result = vkAllocateMemory(m_Vulkan.vkbDevice.device, &allocInfo, nullptr, &m_Swapchain.indexBuffer.memory);

        if (result != VK_SUCCESS) {
            throw Exceptions::EstException("Failed to allocate index buffer memory");
        }

        vkBindBufferMemory(m_Vulkan.vkbDevice.device, m_Swapchain.indexBuffer.buffer, m_Swapchain.indexBuffer.memory, 0);

        vkMapMemory(m_Vulkan.vkbDevice.device, m_Swapchain.indexBuffer.memory, 0, indicies, 0, reinterpret_cast<void **>(&m_DrawData.indexPtr));
    }

    {
        for (int i = 0; i < backupVertices.size(); i++) {
            m_DrawData.vertexPtr[i] = backupVertices[i];
        }

        for (int i = 0; i < backupIndex.size(); i++) {
            m_DrawData.indexPtr[i] = backupIndex[i];
        }
    }
}

void Vulkan::DestroyBuffers()
{
    if (m_Swapchain.vertexBuffer.buffer != VK_NULL_HANDLE) {
        vkUnmapMemory(m_Vulkan.vkbDevice.device, m_Swapchain.vertexBuffer.memory);

        vkDestroyBuffer(m_Vulkan.vkbDevice.device, m_Swapchain.vertexBuffer.buffer, nullptr);
        vkFreeMemory(m_Vulkan.vkbDevice.device, m_Swapchain.vertexBuffer.memory, nullptr);
    }

    if (m_Swapchain.indexBuffer.buffer != VK_NULL_HANDLE) {
        vkUnmapMemory(m_Vulkan.vkbDevice.device, m_Swapchain.indexBuffer.memory);

        vkDestroyBuffer(m_Vulkan.vkbDevice.device, m_Swapchain.indexBuffer.buffer, nullptr);
        vkFreeMemory(m_Vulkan.vkbDevice.device, m_Swapchain.indexBuffer.memory, nullptr);
    }
}

VulkanDescriptor *Vulkan::CreateDescriptor()
{
    auto descriptor = std::make_unique<VulkanDescriptor>();
    descriptor->Id = ++m_DescriptorId;

    m_Descriptors.push_back(std::move(descriptor));
    return m_Descriptors.back().get();
}

void Vulkan::DestroyDescriptor(VulkanDescriptor *descriptor, bool _delete)
{
    auto device = m_Vulkan.vkbDevice.device;
    auto descriptorPool = m_Vulkan.descriptorPool;
    auto uploadBufferMemory = descriptor->UploadBufferMemory;
    auto uploadBuffer = descriptor->UploadBuffer;
    auto sampler = descriptor->Sampler;
    auto imageView = descriptor->ImageView;
    auto image = descriptor->Image;
    auto imageMemory = descriptor->ImageMemory;
    auto vkId = descriptor->VkId;

    m_PerFrameDeletionQueue.push_function([=] {
        vkFreeMemory(device, uploadBufferMemory, nullptr);
        vkDestroyBuffer(device, uploadBuffer, nullptr);
        vkDestroySampler(device, sampler, nullptr);
        vkDestroyImageView(device, imageView, nullptr);
        vkDestroyImage(device, image, nullptr);
        vkFreeMemory(device, imageMemory, nullptr);
        // vkFreeDescriptorSets(device, descriptorPool, 1, &vkId);

        ImGui_ImplVulkan_RemoveTexture(vkId);
    });

    auto it = std::find_if(m_Descriptors.begin(), m_Descriptors.end(), [descriptor](auto &item) {
        return item.get()->Id == descriptor->Id;
    });

    if (_delete) {
        if (it != m_Descriptors.end()) {
            m_Descriptors.erase(it);
        }
    }
}

VulkanObject *Vulkan::GetVulkanObject()
{
    return &m_Vulkan;
}

VulkanSwapChain *Vulkan::GetSwapchain()
{
    return &m_Swapchain;
}

void Vulkan::SetClearColor(glm::vec4 color)
{
}

void Vulkan::SetClearDepth(float depth)
{
}

void Vulkan::SetClearStencil(uint32_t stencil)
{
}

BlendHandle Vulkan::CreateBlendState(TextureBlendInfo blendInfo)
{
    VkResult result;

    if (m_Swapchain.pipelineLayout == VK_NULL_HANDLE) {
        VkDescriptorSetLayout image_descriptor_layout;
        {
            VkDescriptorSetLayoutBinding binding[1] = {};
            binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding[0].descriptorCount = 1;
            binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            VkDescriptorSetLayoutCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.bindingCount = 1;
            info.pBindings = binding;

            result = vkCreateDescriptorSetLayout(m_Vulkan.vkbDevice.device, &info, nullptr, &image_descriptor_layout);

            if (result != VK_SUCCESS) {
                throw Exceptions::EstException("Failed to create descriptor set layout");
            }
        }

        VkPipelineLayout pipeline_layout;
        {
            VkPushConstantRange push_constants[1] = {};
            push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            push_constants[0].offset = 0;
            push_constants[0].size = sizeof(PushConstant);
            VkDescriptorSetLayout      set_layout[1] = { image_descriptor_layout };
            VkPipelineLayoutCreateInfo layout_info = {};
            layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layout_info.setLayoutCount = 1;
            layout_info.pSetLayouts = set_layout;
            layout_info.pushConstantRangeCount = 1;
            layout_info.pPushConstantRanges = push_constants;

            result = vkCreatePipelineLayout(m_Vulkan.vkbDevice.device, &layout_info, nullptr, &pipeline_layout);

            if (result != VK_SUCCESS) {
                throw Exceptions::EstException("Failed to create pipeline layout");
            }
        }

        m_Swapchain.pipelineLayout = pipeline_layout;
        m_DeletionQueue.push_function([=] {
            vkDestroyPipelineLayout(m_Vulkan.vkbDevice.device, m_Swapchain.pipelineLayout, nullptr);
            vkDestroyDescriptorSetLayout(m_Vulkan.vkbDevice.device, image_descriptor_layout, nullptr);
        });
    }

    std::unordered_map<ShaderFragmentType, std::pair<VkShaderModule, VkShaderModule>> shaders = {
        { ShaderFragmentType::Image, { m_Vulkan.vertShaderModule, m_Vulkan.imageFragShaderModule } },
        { ShaderFragmentType::Solid, { m_Vulkan.vertShaderModule, m_Vulkan.solidFragShaderModule } }
    };

    BlendHandle handleId = VkBlendOperatioId++;

    VulkanRenderPipeline blendResult = {};
    blendResult.handle = handleId;

    for (auto &[type, shader_pair] : shaders) {
        VkPipelineShaderStageCreateInfo stage[2] = {};
        stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage[0].module = shader_pair.first;
        stage[0].pName = "main";
        stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage[1].module = shader_pair.second;
        stage[1].pName = "main";

        VkVertexInputBindingDescription binding_desc[1] = {};
        binding_desc[0].stride = sizeof(Vertex);
        binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription attribute_desc[3] = {};
        attribute_desc[0].location = 0;
        attribute_desc[0].binding = binding_desc[0].binding;
        attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[0].offset = MY_OFFSETOF(Vertex, pos);
        attribute_desc[1].location = 1;
        attribute_desc[1].binding = binding_desc[0].binding;
        attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[1].offset = MY_OFFSETOF(Vertex, texCoord);
        attribute_desc[2].location = 2;
        attribute_desc[2].binding = binding_desc[0].binding;
        attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attribute_desc[2].offset = MY_OFFSETOF(Vertex, color);
        // attribute_desc[3].location = 3;
        // attribute_desc[3].binding = binding_desc[0].binding;
        // attribute_desc[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        // attribute_desc[3].offset = MY_OFFSETOF(Vertex, cornerRadius);

        VkPipelineVertexInputStateCreateInfo vertex_info = {};
        vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_info.vertexBindingDescriptionCount = 1;
        vertex_info.pVertexBindingDescriptions = binding_desc;
        vertex_info.vertexAttributeDescriptionCount = sizeof(attribute_desc) / sizeof(attribute_desc[0]);
        vertex_info.pVertexAttributeDescriptions = attribute_desc;

        VkPipelineInputAssemblyStateCreateInfo ia_info = {};
        ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        ia_info.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewport_info = {};
        viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_info.viewportCount = 1;
        viewport_info.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo raster_info = {};
        raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        raster_info.polygonMode = VK_POLYGON_MODE_FILL;
        raster_info.cullMode = VK_CULL_MODE_NONE;
        raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        raster_info.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo ms_info = {};
        ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms_info.rasterizationSamples = m_Vulkan.MSAASampleCount;
        // ms_info.sampleShadingEnable = VK_TRUE;

        VkPipelineDepthStencilStateCreateInfo depth_info = {};
        depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        VkPipelineColorBlendStateCreateInfo blend_info = {};
        blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend_info.attachmentCount = 1;

        VkDynamicState                   dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic_state = {};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = (uint32_t)(sizeof(dynamic_states) / sizeof(dynamic_states[0]));
        dynamic_state.pDynamicStates = dynamic_states;

        VkPipeline pipeline;
        {
            VkPipelineColorBlendAttachmentState color_attachment[1] = {};
            if (blendInfo.Enable) {
                color_attachment[0].blendEnable = VK_TRUE;
                color_attachment[0].srcColorBlendFactor = static_cast<VkBlendFactor>(blendInfo.SrcColor);
                color_attachment[0].dstColorBlendFactor = static_cast<VkBlendFactor>(blendInfo.DstColor);
                color_attachment[0].colorBlendOp = static_cast<VkBlendOp>(blendInfo.ColorOp);
                color_attachment[0].srcAlphaBlendFactor = static_cast<VkBlendFactor>(blendInfo.SrcAlpha);
                color_attachment[0].dstAlphaBlendFactor = static_cast<VkBlendFactor>(blendInfo.DstAlpha);
                color_attachment[0].alphaBlendOp = static_cast<VkBlendOp>(blendInfo.AlphaOp);
                color_attachment[0].colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            } else {
                color_attachment[0].blendEnable = VK_FALSE;
            }

            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

            blend_info.pAttachments = color_attachment;

            VkGraphicsPipelineCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            info.flags = 0;
            info.stageCount = 2;
            info.pStages = stage;
            info.pVertexInputState = &vertex_info;
            info.pInputAssemblyState = &ia_info;
            info.pViewportState = &viewport_info;
            info.pRasterizationState = &raster_info;
            info.pMultisampleState = &ms_info;
            info.pDepthStencilState = &depth_info;
            info.pColorBlendState = &blend_info;
            info.pDynamicState = &dynamic_state;
            info.layout = m_Swapchain.pipelineLayout;
            info.renderPass = m_Swapchain.renderpass;
            info.subpass = 0;

            result = vkCreateGraphicsPipelines(m_Vulkan.vkbDevice.device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);

            if (result != VK_SUCCESS) {
                throw Exceptions::EstException("Failed to create graphics pipeline");
            }
        }

        blendResult.pipelines[type] = pipeline;

        m_DeletionQueue.push_function([=] {
            vkDestroyPipeline(m_Vulkan.vkbDevice.device, pipeline, nullptr);
        });
    }

    m_BlendStates[handleId] = std::move(blendResult);
    return handleId;
}

void Vulkan::ImGui_Init()
{
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiPool;
    auto             result = vkCreateDescriptorPool(
        m_Vulkan.vkbDevice.device,
        &pool_info,
        nullptr,
        &imguiPool);

    if (result != VK_SUCCESS) {
        throw Exceptions::EstException("Failed to init imgui descriptor pool");
    }

    ImGui::CreateContext();

    auto window = Graphics::NativeWindow::Get();
    ImGui_ImplSDL2_InitForVulkan(window->GetWindow());
    window->AddSDLCallback([=](SDL_Event &ev) {
        ImGui_ImplSDL2_ProcessEvent(&ev);
    });

    auto &IO = ImGui::GetIO();
    auto  size = window->GetWindowSize();
    IO.DisplaySize = ImVec2((float)size.Width, (float)size.Height);
    IO.IniFilename = NULL;

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_Vulkan.vkbInstance.instance;
    init_info.PhysicalDevice = m_Vulkan.vkbDevice.physical_device;
    init_info.Device = m_Vulkan.vkbDevice.device;
    init_info.Queue = m_Vulkan.graphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = m_Vulkan.MSAASampleCount;

    ImGui_ImplVulkan_Init(&init_info, m_Swapchain.renderpass);

    // Upload texture
    ImmediateSubmit([=](VkCommandBuffer cmd) {
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
    });

    m_Imgui.imguiPool = imguiPool;
}

void Vulkan::ImGui_DeInit()
{
    if (m_Imgui.imguiPool == VK_NULL_HANDLE) {
        return;
    }

    ImGui_ImplVulkan_DestroyFontUploadObjects();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    vkDestroyDescriptorPool(m_Vulkan.vkbDevice.device, m_Imgui.imguiPool, nullptr);

    ImGui::DestroyContext();
}

void Vulkan::ImGui_NewFrame()
{
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();

    m_HasImgui = true;
}

void Vulkan::ImGui_EndFrame()
{
    ImGui::EndFrame();
    ImGui::Render();
}

void Vulkan::SetVSync(bool enabled)
{
    bool lastValue = m_VSync;
    m_VSync = enabled;

    if (lastValue != enabled) {
        m_SwapchainReady = false;
    }
}