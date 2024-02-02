/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __VULKANBACKEND_H_
#define __VULKANBACKEND_H_

#include <deque>
#include <functional>
#include <memory>
#include <unordered_map>

#include "./Volk/volk.h"
#include "./VulkanBootstrap/VkBootstrap.h"
#include "VulkanDescriptor.h"
#include <Graphics/GraphicsBackendBase.h>

struct DeletionQueue
{
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()> &&function)
    {
        deletors.push_back(function);
    }

    void flush()
    {
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)();
        }

        deletors.clear();
    }
};

namespace Graphics {
    namespace Backends {
        struct VulkanObject
        {
            vkb::Instance vkbInstance;
            vkb::Device   vkbDevice;
            VkSurfaceKHR  surface;

            VkQueue  graphicsQueue;
            uint32_t graphicsQueueFamily;

            VkFormat depthFormat;
            VkFormat swapchainFormat;

            VkDescriptorPool      descriptorPool;
            VkDescriptorSetLayout descriptorSetLayout;

            VkShaderModule vertShaderModule;
            VkShaderModule solidFragShaderModule;
            VkShaderModule imageFragShaderModule;

            VkSampleCountFlagBits MSAASampleCount;
        };

        struct MultiSamplingTarget
        {
            struct
            {
                VkImage        image;
                VkImageView    view;
                VkDeviceMemory memory;
            } color;

            struct
            {
                VkImage        image;
                VkImageView    view;
                VkDeviceMemory memory;
            } depth;
        };

        struct VulkanBuffer
        {
            VkBuffer       buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;
        };

        struct VulkanFrame
        {
            VkSemaphore presentSemaphore;
            VkSemaphore renderSemaphore;
            VkFence     renderFence;

            VkCommandPool   commandPool;
            VkCommandBuffer commandBuffer;

            bool isValid;
        };

        struct VulkanSwapChain
        {
            std::vector<VkFramebuffer> framebuffers;
            std::vector<VkImageView>   imageViews;
            std::vector<VkImage>       images;

            std::vector<VulkanFrame> frames;
            VulkanFrame              uploadContext;
            VkPipelineLayout         pipelineLayout;

            uint32_t maxVertexBufferSize;
            uint32_t maxIndexBufferSize;

            VulkanBuffer vertexBuffer;
            VulkanBuffer indexBuffer;

            VkImage        depthImage;
            VkImageView    depthImageView;
            VkDeviceMemory depthImageMemory;

            vkb::Swapchain swapchain;
            VkRenderPass   renderpass;
            uint32_t       imageCount;
            uint32_t       swapchainIndex;
        };

        struct VulkanRenderPipeline
        {
            BlendHandle                                        handle;
            std::unordered_map<ShaderFragmentType, VkPipeline> pipelines;
        };

        struct VulkanImGui
        {
            VkDescriptorPool imguiPool;
        };

        struct VulkanDrawData
        {
            Vertex   *vertexPtr;
            uint16_t *indexPtr;

            uint32_t vertexSize;
            uint32_t indiceSize;

            void Reset()
            {
                vertexSize = 0;
                indiceSize = 0;
            }
        };

        struct VulkanDrawItem
        {
            uint32_t           count;
            uint16_t           instanceCount;
            ShaderFragmentType type;
            BlendHandle        blend;
            const void        *image;

            Rect      clipRect;
            glm::vec2 uiSize;
            glm::vec4 uiRadius;
        };

        class Vulkan : public Base
        {
        public:
            virtual ~Vulkan() = default;

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

            /* Internal */
            VulkanDescriptor *CreateDescriptor();
            void              DestroyDescriptor(VulkanDescriptor *descriptor, bool _delete = true);

            VulkanObject    *GetVulkanObject();
            VulkanSwapChain *GetSwapchain();

            void ImmediateSubmit(std::function<void(VkCommandBuffer)> &&function);

        private:
            void CreateInstance();
            void CreateRenderpass();
            void InitFramebuffers();
            void InitCommands();
            void InitSyncStructures();
            void InitDescriptors();
            void InitShaders();
            void InitPipeline();
            void InitMultiSampling();
            bool InitSwapchain();
            void DestroyBuffers();

            void         FlushQueue();
            void         ResizeBuffer(VkDeviceSize vertices, VkDeviceSize indices);
            VulkanFrame &GetCurrentFrame();
            VulkanFrame &GetLastFrame();

            VulkanObject        m_Vulkan;
            VulkanSwapChain     m_Swapchain;
            VulkanImGui         m_Imgui;
            VulkanDrawData      m_DrawData;
            MultiSamplingTarget m_MultiSamplingTarget;

            // Pending infos
            std::vector<VulkanDrawItem> submitInfos;

            // OnExit program clean up
            DeletionQueue m_DeletionQueue;

            // OnFrame program clean up, like deleting texture
            DeletionQueue m_PerFrameDeletionQueue;

            // OnSwapchain program clean up, like re-creating swapchain
            DeletionQueue m_SwapchainDeletionQueue;

            bool m_SwapchainReady;
            bool m_Initialized;
            bool m_FrameBegin;
            bool m_HasImgui = false;
            bool m_VSync = false;

            uint32_t m_CurrentFrame = 0;

            // Descriptor, for auto cleanup
            std::vector<std::unique_ptr<VulkanDescriptor>> m_Descriptors;
            uint32_t                                       m_DescriptorId = 0;

            // Alpha blending
            std::unordered_map<BlendHandle, VulkanRenderPipeline> m_BlendStates;
        };
    } // namespace Backends
} // namespace Graphics

#endif