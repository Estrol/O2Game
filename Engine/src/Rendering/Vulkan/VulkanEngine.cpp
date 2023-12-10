
#include "Rendering/Vulkan/VulkanEngine.h"

#include "Rendering/Vulkan/VkBootstrap/VkBootstrap.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <fstream>
#include <iostream>

#include "../../Data/Imgui/imgui_impl_sdl2.h"
#include "../../Data/Imgui/imgui_impl_vulkan.h"
#include "Exception/SDLException.h"
#include "Rendering/Vulkan/Texture2DVulkan.h"
#include "vkinit.h"
#include <Logs.h>

#include "./Shaders/color.spv.h"
#include "./Shaders/position.spv.h"

#if _DEBUG
constexpr bool bUseValidationLayers = true;
#else
constexpr bool bUseValidationLayers = false;
#endif

// we want to immediately abort when there is an error.
// In normal engines this would give an error message to the user, or perform a dump of state.
using namespace std;
#if _DEBUG && _WIN32
#define _DEBUGBREAK_WIN32 __debugbreak();
#else
#define _DEBUGBREAK_WIN32
#endif

#define VK_CHECK(x)                                                          \
    do {                                                                     \
        VkResult err = x;                                                    \
        if (err) {                                                           \
            _DEBUGBREAK_WIN32                                                \
            throw std::runtime_error("VulkanError: " + std::to_string(err)); \
        }                                                                    \
    } while (0)

// GCC/G++, disable G3B9DA338
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#endif

VulkanEngine::~VulkanEngine()
{
    cleanup();
}

VulkanEngine *VulkanEngine::GetInstance()
{
    if (m_instance == nullptr) {
        m_instance = new VulkanEngine();
    }

    return m_instance;
}

VulkanEngine *VulkanEngine::m_instance = nullptr;

void VulkanEngine::Release()
{
    if (m_instance != nullptr) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void VulkanEngine::init(SDL_Window *window, int width, int height)
{
    _window = window;

    _windowExtent.width = width;
    _windowExtent.height = height;

    init_vulkan();
    init_swapchain();
    init_default_renderpass();
    init_framebuffers();
    init_commands();
    init_sync_structures();
    init_descriptors();
    init_shaders();
    init_pipeline();
    imgui_init();

    _isInitialized = true;
}
void VulkanEngine::cleanup()
{
    if (_isInitialized) {
        vkDeviceWaitIdle(_device);

        vkTexture::Cleanup();

        _swapChainQueue.flush();
        _mainDeletionQueue.flush();

        for (int i = 0; i < _swapchainImageViews.size(); i++) {
            vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
        }

        // vkb::destroy_swapchain(_swapchainData);
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
        vkDestroySurfaceKHR(_instance, _surface, nullptr);

        vkDestroyDevice(_device, nullptr);
        vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
        vkDestroyInstance(_instance, nullptr);
    }
}

bool minized(SDL_Window *wnd)
{
    return SDL_GetWindowFlags(wnd) & SDL_WINDOW_MINIMIZED;
}

void VulkanEngine::begin()
{
    if (_swapChainOutdated || _swapchain == VK_NULL_HANDLE)
        return;

    if (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED)
        return;

    auto waitdevice = vkDeviceWaitIdle(_device);
    VK_CHECK(waitdevice);

    auto waitFence = vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, static_cast<int64_t>(1e+9));
    if (waitFence == VK_TIMEOUT) {
        return;
    }

    VK_CHECK(waitFence);

    auto requestResult = vkAcquireNextImageKHR(_device, _swapchain, static_cast<int64_t>(1e+9), get_current_frame()._presentSemaphore, nullptr, &_swapchainNumber);
    if (requestResult == VK_ERROR_OUT_OF_DATE_KHR) {
        _swapChainOutdated = true;
        return;
    } else {
        VK_CHECK(requestResult);
    }

    VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

    VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

    _perFrameDeletionQueue.flush();

    if (!get_current_frame().IsValid)
        return;
    VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    VkClearValue clearValue;
    clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;

    VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_renderPass, _windowExtent, _framebuffers[_swapchainNumber]);

    rpInfo.clearValueCount = 2;

    VkClearValue clearValues[] = { clearValue, depthClear };

    rpInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanEngine::end()
{
    if (_swapChainOutdated || _swapchain == VK_NULL_HANDLE)
        return;

    if (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED)
        return;

    if (!get_current_frame().IsValid)
        return;
    VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

    vkCmdEndRenderPass(cmd);
    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo         submit = vkinit::submit_info(&cmd);
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

    VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

    VkPresentInfoKHR presentInfo = vkinit::present_info();

    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &_swapchainNumber;

    auto presentResult = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        _swapChainOutdated = true;
    } else {
        VK_CHECK(presentResult);
    }

    _frameNumber++;
}

void ImGui_CheckResult(VkResult e)
{
#if _DEBUG
    if (e != VK_SUCCESS) {
        DebugBreak();
    }

#endif
    VK_CHECK(e);
}

void VulkanEngine::imgui_init()
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
    VK_CHECK(vkCreateDescriptorPool(_device, &pool_info, nullptr, &imguiPool));

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForVulkan(_window);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = _instance;
    init_info.PhysicalDevice = _chosenGPU;
    init_info.Device = _device;
    init_info.Queue = _graphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = ImGui_CheckResult;

    ImGui_ImplVulkan_Init(&init_info, _renderPass);

    _mainDeletionQueue.push_function([=]() {
        vkDestroyDescriptorPool(_device, imguiPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
    });
}

void VulkanEngine::imgui_begin()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame(_window);
}

void VulkanEngine::imgui_end()
{
    ImGui::Render();
    ImGui_ImplSDL2_ResetFrame();
    ImGui_ImplVulkan_ResetFrame();

    if (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED)
        return;
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), get_current_frame()._mainCommandBuffer);
}

void VulkanEngine::set_vsync(bool v)
{
    m_vsync = v;
    _swapChainOutdated = true;
}

AllocatedBuffer VulkanEngine::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    AllocatedBuffer newBuffer;
    VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
                             &newBuffer._buffer,
                             &newBuffer._allocation,
                             nullptr));

    return newBuffer;
}

size_t VulkanEngine::pad_uniform_buffer_size(size_t originalSize)
{
    size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = originalSize;
    if (minUboAlignment > 0) {
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return alignedSize;
}

FrameData &VulkanEngine::get_current_frame()
{
    return _frames[_frameNumber % FRAME_OVERLAP];
}

FrameData &VulkanEngine::get_last_frame()
{
    return _frames[(_frameNumber - 1) % FRAME_OVERLAP];
}

void VulkanEngine::init_vulkan()
{
    if (volkInitialize() != VK_SUCCESS) {
        throw std::runtime_error("Vulkan not supported on this system!");
    }

    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name("EstEngine")
                        .request_validation_layers(bUseValidationLayers)
                        .use_default_debug_messenger()
                        .require_api_version(1, 1, 0)
                        .build();

    vkb::Instance vkb_inst = inst_ret.value();

    _instance = vkb_inst.instance;
    _debug_messenger = vkb_inst.debug_messenger;
    _allocCallback = vkb_inst.allocation_callbacks;

    volkLoadInstance(_instance);

    if (!SDL_Vulkan_CreateSurface(_window, _instance, &_surface)) {
        throw SDLException();
    }

    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    vkb::PhysicalDevice         physicalDevice = selector
                                             .set_minimum_version(1, 1)
                                             .set_surface(_surface)
                                             .select()
                                             .value();

    vkb::DeviceBuilder deviceBuilder{ physicalDevice };

    vkb::Device vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _chosenGPU = physicalDevice.physical_device;

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    VmaVulkanFunctions vma_vulkan_func{};
    vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _chosenGPU;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    allocatorInfo.pVulkanFunctions = &vma_vulkan_func;
    vmaCreateAllocator(&allocatorInfo, &_allocator);

    _mainDeletionQueue.push_function([&]() {
        vmaDestroyAllocator(_allocator);
    });

    vkGetPhysicalDeviceProperties(_chosenGPU, &_gpuProperties);

    Logs::Puts("[VulkanRenderer] The gpu has a minimum buffer alignement of %d", _gpuProperties.limits.minUniformBufferOffsetAlignment);
}

bool VulkanEngine::init_swapchain()
{
    vkb::SwapchainBuilder swapchainBuilder{ _chosenGPU, _device, _surface };

    SDL_GetWindowSize(_window, (int *)&_windowExtent.width, (int *)&_windowExtent.height);
    if (_windowExtent.width == 0 || _windowExtent.height == 0) {
        return false;
    }

    swapchainBuilder
        .set_desired_format({ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_old_swapchain(_swapchainData)
        .set_desired_extent(_windowExtent.width, _windowExtent.height);

    if (m_vsync) {
        swapchainBuilder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    } else {
        swapchainBuilder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
    }

    vkb::Result vkbSwapchainResult = swapchainBuilder.build();

    if (!vkbSwapchainResult) {
        _swapchain = VK_NULL_HANDLE;
        _swapchainData.swapchain = VK_NULL_HANDLE;
        vkb::destroy_swapchain(_swapchainData);

        return false;
    }

    vkb::destroy_swapchain(_swapchainData);
    _swapchainData = vkbSwapchainResult.value();

    vkb::Swapchain vkbSwapchain = vkbSwapchainResult.value();

    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();

    _swachainImageFormat = vkbSwapchain.image_format;

    VkExtent3D depthImageExtent = {
        _windowExtent.width,
        _windowExtent.height,
        1
    };

    _depthFormat = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo dimg_info = vkinit::image_create_info(_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(_allocator, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);

    VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);
    ;

    VK_CHECK(vkCreateImageView(_device, &dview_info, nullptr, &_depthImageView));

    _swapChainQueue.push_function([=]() {
        vkDestroyImageView(_device, _depthImageView, nullptr);
        vmaDestroyImage(_allocator, _depthImage._image, _depthImage._allocation);

        _depthImageView = VK_NULL_HANDLE;
        _depthImage._image = VK_NULL_HANDLE;
    });

    return true;
}

void VulkanEngine::init_default_renderpass()
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = _swachainImageFormat;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment = {};
    depth_attachment.flags = 0;
    depth_attachment.format = _depthFormat;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency depth_dependency = {};
    depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    depth_dependency.dstSubpass = 0;
    depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.srcAccessMask = 0;
    depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency dependencies[2] = { dependency, depth_dependency };

    VkAttachmentDescription attachments[2] = { color_attachment, depth_attachment };

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 2;
    render_pass_info.pAttachments = &attachments[0];
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 2;
    render_pass_info.pDependencies = &dependencies[0];

    VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderPass));

    _mainDeletionQueue.push_function([=]() {
        vkDestroyRenderPass(_device, _renderPass, nullptr);
    });
}

bool VulkanEngine::init_framebuffers()
{
    int width = 0, height = 0;
    SDL_GetWindowSize(_window, &width, &height);

    if (_windowExtent.width != width || _windowExtent.height != height) {
        return false;
    }

    VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(_renderPass, _windowExtent);

    const uint32_t swapchain_imagecount = (uint32_t)_swapchainImages.size();
    _framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

    for (uint32_t i = 0; i < swapchain_imagecount; i++) {
        VkImageView attachments[2] = {};
        attachments[0] = _swapchainImageViews[i];
        attachments[1] = _depthImageView;

        fb_info.pAttachments = attachments;
        fb_info.attachmentCount = 2;
        VK_CHECK(vkCreateFramebuffer(_device, &fb_info, nullptr, &_framebuffers[i]));

        _swapChainQueue.push_function([=]() {
            vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
            vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);

            _framebuffers[i] = VK_NULL_HANDLE;
            _swapchainImageViews[i] = VK_NULL_HANDLE;
        });
    }

    return true;
}

void VulkanEngine::init_commands()
{
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

        VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
        _frames[i].IsValid = true;

        _mainDeletionQueue.push_function([=]() {
            vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
            _frames[i].IsValid = false;
        });
    }

    VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily);
    VK_CHECK(vkCreateCommandPool(_device, &uploadCommandPoolInfo, nullptr, &_uploadContext._commandPool));

    _mainDeletionQueue.push_function([=]() {
        vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr);
    });

    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_uploadContext._commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_uploadContext._commandBuffer));

    const int MAX_OBJECTS = 50000;
    _maxVertexBufferSize = sizeof(ImDrawVert) * MAX_OBJECTS;
    _maxIndexBufferSize = sizeof(int) * MAX_OBJECTS;

    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = _maxVertexBufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateBuffer(_device, &bufferInfo, nullptr, &_vertexBuffer);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(_device, _vertexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vkTexture::FindMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(_device, &allocInfo, nullptr, &_vertexBufferMemory);

        vkBindBufferMemory(_device, _vertexBuffer, _vertexBufferMemory, 0);
    }

    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = _maxIndexBufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateBuffer(_device, &bufferInfo, nullptr, &_indexBuffer);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(_device, _indexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = vkTexture::FindMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(_device, &allocInfo, nullptr, &_indexBufferMemory);

        vkBindBufferMemory(_device, _indexBuffer, _indexBufferMemory, 0);
    }

    _mainDeletionQueue.push_function([=]() {
        if (_vertexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(_device, _vertexBuffer, nullptr);
            vkFreeMemory(_device, _vertexBufferMemory, nullptr);
            _vertexBuffer = VK_NULL_HANDLE;
            _vertexBufferMemory = VK_NULL_HANDLE;
        }

        if (_indexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(_device, _indexBuffer, nullptr);
            vkFreeMemory(_device, _indexBufferMemory, nullptr);
            _indexBuffer = VK_NULL_HANDLE;
            _indexBufferMemory = VK_NULL_HANDLE;
        }
    });
}

void VulkanEngine::init_sync_structures()
{
    VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

    for (int i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));
        _mainDeletionQueue.push_function([=]() {
            vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
        });

        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._presentSemaphore));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));

        _mainDeletionQueue.push_function([=]() {
            vkDestroySemaphore(_device, _frames[i]._presentSemaphore, nullptr);
            vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
        });
    }

    VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();

    VK_CHECK(vkCreateFence(_device, &uploadFenceCreateInfo, nullptr, &_uploadContext._uploadFence));
    _mainDeletionQueue.push_function([=]() {
        vkDestroyFence(_device, _uploadContext._uploadFence, nullptr);
    });
}

void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function)
{
    if (_swapchain == VK_NULL_HANDLE) {
        throw std::runtime_error("NO_SWAP_CHAIN");
    }

    VkCommandBuffer          cmd = _uploadContext._commandBuffer;
    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submit = vkinit::submit_info(&cmd);

    VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _uploadContext._uploadFence));

    vkWaitForFences(_device, 1, &_uploadContext._uploadFence, true, 9999999999);
    vkResetFences(_device, 1, &_uploadContext._uploadFence);

    vkResetCommandPool(_device, _uploadContext._commandPool, 0);
}

void VulkanEngine::queue_submit(SubmitQueueInfo info)
{
    _queueInfos.emplace_back(std::move(info));
}

void VulkanEngine::flush_queue()
{
    if (_queueInfos.size() <= 0) {
        return;
    }

    int width, height;
    SDL_GetWindowSize(_window, &width, &height);

    if (_swapChainOutdated || _swapchain == VK_NULL_HANDLE || width <= 0 || height <= 0 || SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED) {
        _queueInfos.clear();
        return;
    }

    VkDeviceSize vertex_size = 0;
    VkDeviceSize indices_size = 0;
    for (auto &info : _queueInfos) {
        vertex_size += info.vertices.size() * sizeof(info.vertices[0]);
        indices_size += info.indices.size() * sizeof(info.indices[0]);
    }

    vertex_size = std::clamp((uint64_t)vertex_size, (uint64_t)0, _maxVertexBufferSize);
    indices_size = std::clamp((uint64_t)indices_size, (uint64_t)0, _maxIndexBufferSize);

    void *data;
    VK_CHECK(vkMapMemory(_device, _vertexBufferMemory, 0, vertex_size, 0, &data));

    void *data2;
    VK_CHECK(vkMapMemory(_device, _indexBufferMemory, 0, indices_size, 0, &data2));

    VkDeviceSize offset = 0;
    for (auto &info : _queueInfos) {
        if (offset >= _maxVertexBufferSize) {
            continue;
        }

        memcpy((char *)data + offset, info.vertices.data(), info.vertices.size() * sizeof(info.vertices[0]));
        offset += info.vertices.size() * sizeof(info.vertices[0]);
    }

    offset = 0;
    for (auto &info : _queueInfos) {
        memcpy((char *)data2 + offset, info.indices.data(), info.indices.size() * sizeof(info.indices[0]));
        offset += info.indices.size() * sizeof(info.indices[0]);
    }

    vkUnmapMemory(_device, _vertexBufferMemory);
    vkUnmapMemory(_device, _indexBufferMemory);

    if (!get_current_frame().IsValid)
        return;
    auto cmd = get_current_frame()._mainCommandBuffer;

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)_windowExtent.width;
    viewport.height = (float)_windowExtent.height;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, &_vertexBuffer, offsets);
    vkCmdBindIndexBuffer(cmd, _indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    float scale[2];
    scale[0] = 2.0f / _windowExtent.width;
    scale[1] = 2.0f / _windowExtent.height;

    float translate[2];
    translate[0] = -1.0f;
    translate[1] = -1.0f;

    int currentVertIndex = 0; // vertex
    int currentIndiIndex = 0; // indicies

    for (auto &info : _queueInfos) {
        auto pipeline = _pipeline_layout;
        auto graphics = _graphics_pipeline;

        if (!info.AlphaBlend) {
            pipeline = _pipeline_layout_non_blend;
            graphics = _graphics_pipeline_non_blend;
        }

        vkCmdPushConstants(cmd, pipeline, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 2, scale);
        vkCmdPushConstants(cmd, pipeline, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline, 0, 1, &info.descriptor, 0, nullptr);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics);

        vkCmdSetScissor(cmd, 0, 1, &info.scissor);

        vkCmdDrawIndexed(cmd, (uint32_t)info.vertices.size(), 1, currentIndiIndex, currentVertIndex, 0);
        currentVertIndex += (int)info.vertices.size();
        currentIndiIndex += (int)info.indices.size();
    }

    _queueInfos.clear();
}

void VulkanEngine::init_descriptors()
{
    std::vector<VkDescriptorPoolSize> sizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = 10;
    pool_info.poolSizeCount = (uint32_t)sizes.size();
    pool_info.pPoolSizes = sizes.data();

    vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptorPool);

    VkDescriptorSetLayoutBinding cameraBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    VkDescriptorSetLayoutBinding sceneBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);

    VkDescriptorSetLayoutBinding bindings[] = { cameraBind, sceneBind };

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.bindingCount = 2;
    setinfo.flags = 0;
    setinfo.pNext = nullptr;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = bindings;

    vkCreateDescriptorSetLayout(_device, &setinfo, nullptr, &_globalSetLayout);

    VkDescriptorSetLayoutBinding objectBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    VkDescriptorSetLayoutCreateInfo set2info = {};
    set2info.bindingCount = 1;
    set2info.flags = 0;
    set2info.pNext = nullptr;
    set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set2info.pBindings = &objectBind;

    vkCreateDescriptorSetLayout(_device, &set2info, nullptr, &_objectSetLayout);

    VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

    VkDescriptorSetLayoutCreateInfo set3info = {};
    set3info.bindingCount = 1;
    set3info.flags = 0;
    set3info.pNext = nullptr;
    set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set3info.pBindings = &textureBind;

    vkCreateDescriptorSetLayout(_device, &set3info, nullptr, &_singleTextureSetLayout);

    _mainDeletionQueue.push_function([&]() {
        vkDestroyDescriptorSetLayout(_device, _objectSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(_device, _globalSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(_device, _singleTextureSetLayout, nullptr);

        vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    });
}

void VulkanEngine::init_shaders()
{
    VkShaderModule vert, frag;

    VkShaderModuleCreateInfo vert_info = {};
    vert_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vert_info.codeSize = sizeof(__glsl_vert);
    vert_info.pCode = (uint32_t *)__glsl_vert;
    VK_CHECK(vkCreateShaderModule(_device, &vert_info, nullptr, &vert));

    VkShaderModuleCreateInfo frag_info = {};
    frag_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    frag_info.codeSize = sizeof(__glsl_frag);
    frag_info.pCode = (uint32_t *)__glsl_frag;
    VK_CHECK(vkCreateShaderModule(_device, &frag_info, nullptr, &frag));

    _vert_shader = vert;
    _frag_shader = frag;

    _mainDeletionQueue.push_function([=]() {
        vkDestroyShaderModule(_device, _vert_shader, nullptr);
        vkDestroyShaderModule(_device, _frag_shader, nullptr);
    });
}

void VulkanEngine::init_pipeline()
{
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
        VK_CHECK(vkCreateDescriptorSetLayout(_device, &info, nullptr, &image_descriptor_layout));
    }

    // Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection matrix
    VkPipelineLayout pipeline_layout_non_blend;
    {
        VkPushConstantRange push_constants[1] = {};
        push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        push_constants[0].offset = sizeof(float) * 0;
        push_constants[0].size = sizeof(float) * 4;
        VkDescriptorSetLayout      set_layout[1] = { image_descriptor_layout };
        VkPipelineLayoutCreateInfo layout_info = {};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_info.setLayoutCount = 1;
        layout_info.pSetLayouts = set_layout;
        layout_info.pushConstantRangeCount = 1;
        layout_info.pPushConstantRanges = push_constants;
        VK_CHECK(vkCreatePipelineLayout(_device, &layout_info, nullptr, &pipeline_layout_non_blend));
    }

    VkPipelineLayout pipeline_layout;
    {
        VkPushConstantRange push_constants[1] = {};
        push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        push_constants[0].offset = sizeof(float) * 0;
        push_constants[0].size = sizeof(float) * 4;
        VkDescriptorSetLayout      set_layout[1] = { image_descriptor_layout };
        VkPipelineLayoutCreateInfo layout_info = {};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_info.setLayoutCount = 1;
        layout_info.pSetLayouts = set_layout;
        layout_info.pushConstantRangeCount = 1;
        layout_info.pPushConstantRanges = push_constants;
        VK_CHECK(vkCreatePipelineLayout(_device, &layout_info, nullptr, &pipeline_layout));
    }

    VkPipelineShaderStageCreateInfo stage[2] = {};
    stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage[0].module = _vert_shader;
    stage[0].pName = "main";
    stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage[1].module = _frag_shader;
    stage[1].pName = "main";

    VkVertexInputBindingDescription binding_desc[1] = {};
    binding_desc[0].stride = sizeof(ImDrawVert);
    binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attribute_desc[3] = {};
    attribute_desc[0].location = 0;
    attribute_desc[0].binding = binding_desc[0].binding;
    attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_desc[0].offset = IM_OFFSETOF(ImDrawVert, pos);
    attribute_desc[1].location = 1;
    attribute_desc[1].binding = binding_desc[0].binding;
    attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_desc[1].offset = IM_OFFSETOF(ImDrawVert, uv);
    attribute_desc[2].location = 2;
    attribute_desc[2].binding = binding_desc[0].binding;
    attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    attribute_desc[2].offset = IM_OFFSETOF(ImDrawVert, col);

    VkPipelineVertexInputStateCreateInfo vertex_info = {};
    vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_info.vertexBindingDescriptionCount = 1;
    vertex_info.pVertexBindingDescriptions = binding_desc;
    vertex_info.vertexAttributeDescriptionCount = 3;
    vertex_info.pVertexAttributeDescriptions = attribute_desc;

    VkPipelineInputAssemblyStateCreateInfo ia_info = {};
    ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

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
    ms_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_info = {};
    depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    VkPipelineColorBlendStateCreateInfo blend_info = {};
    blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend_info.attachmentCount = 1;

    VkDynamicState                   dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = (uint32_t)IM_ARRAYSIZE(dynamic_states);
    dynamic_state.pDynamicStates = dynamic_states;

    VkPipeline pipeline_non_alpha_blend;
    {
        VkPipelineColorBlendAttachmentState color_attachment[1] = {};
        color_attachment[0].blendEnable = VK_TRUE;
        color_attachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_attachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_attachment[0].colorBlendOp = VK_BLEND_OP_ADD;
        color_attachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_attachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_attachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
        color_attachment[0].colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

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
        info.layout = pipeline_layout_non_blend;
        info.renderPass = _renderPass;
        info.subpass = 0;

        VK_CHECK(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline_non_alpha_blend));
    }

    VkPipeline pipeline;
    {
        VkPipelineColorBlendAttachmentState color_attachment[1] = {};
        color_attachment[0].blendEnable = VK_TRUE;
        color_attachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_attachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_attachment[0].colorBlendOp = VK_BLEND_OP_ADD;
        color_attachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_attachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_attachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
        color_attachment[0].colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

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
        info.layout = pipeline_layout_non_blend;
        info.renderPass = _renderPass;
        info.subpass = 0;

        VK_CHECK(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline));
    }

    _graphics_pipeline = pipeline;
    _graphics_pipeline_non_blend = pipeline_non_alpha_blend;
    _pipeline_layout = pipeline_layout;
    _pipeline_layout_non_blend = pipeline_layout_non_blend;

    _mainDeletionQueue.push_function([=]() {
        vkDestroyPipeline(_device, _graphics_pipeline_non_blend, nullptr);
        vkDestroyPipelineLayout(_device, _pipeline_layout_non_blend, nullptr);

        vkDestroyPipeline(_device, _graphics_pipeline, nullptr);
        vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);

        vkDestroyDescriptorSetLayout(_device, image_descriptor_layout, nullptr);
    });
}

void VulkanEngine::re_init_swapchains(int width, int height)
{
    if (!_swapChainOutdated) {
        return;
    }

    vkDeviceWaitIdle(_device);
    _swapChainQueue.flush();

    _windowExtent.width = width;
    _windowExtent.height = height;

    VkSwapchainKHR oldSwapchain = _swapchain;

    // _swapchainImageViews
    for (int i = 0; i < _swapchainImageViews.size(); i++) {
        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }

    bool swapchain_recreated = init_swapchain();
    if (!swapchain_recreated) {
        return;
    }

    if (!init_framebuffers()) {
        return;
    }

    Logs::Puts("[VulkanEngine] Swapchain re-create with resolution: %dx%d", width, height);
    _swapChainOutdated = false;
}