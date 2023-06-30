#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <deque>
#include <unordered_map>
#include "vk_mem_alloc/vk_mem_alloc.h"
#include "../Data/WindowsTypes.hpp"

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call functors
		}

		deletors.clear();
	}
};

struct AllocatedImage {
	VkImage _image;
	VmaAllocation _allocation;
};

struct AllocatedBuffer {
	VkBuffer _buffer;
	VmaAllocation _allocation;
};

struct FrameData {
	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;

	DeletionQueue _frameDeletionQueue;

	VkCommandPool _commandPool;
	VkCommandBuffer _mainCommandBuffer;

	AllocatedBuffer cameraBuffer;
	VkDescriptorSet globalDescriptor;

	AllocatedBuffer objectBuffer;
	VkDescriptorSet objectDescriptor;

	bool IsValid;
};

struct UploadContext {
	VkFence _uploadFence;
	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;
};

struct ImDrawVert;

struct SubmitQueueInfo {
	std::vector<ImDrawVert> vertices;
	std::vector<uint16_t> indices;

	VkRect2D scissor;
	bool AlphaBlend;
	VkDescriptorSet descriptor;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine {
public:
	bool _isInitialized{ false };
	bool _swapChainOutdated{ false };
	int _frameNumber{ 0 };
	uint32_t _swapchainNumber{ 0 };

	VkExtent2D _windowExtent{ 1700 , 900 };

	struct SDL_Window* _window{ nullptr };

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkPhysicalDevice _chosenGPU;
	VkDevice _device;

	VkPhysicalDeviceProperties _gpuProperties;

	FrameData _frames[FRAME_OVERLAP];

	VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamily;

	VkRenderPass _renderPass;

	VkSurfaceKHR _surface;
	VkSwapchainKHR _swapchain;
	VkFormat _swachainImageFormat;

	std::vector<VkFramebuffer> _framebuffers;
	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;

	DeletionQueue _mainDeletionQueue;
	DeletionQueue _swapChainQueue;
	VmaAllocator _allocator;

	VkImageView _depthImageView;
	AllocatedImage _depthImage;

	//the format for the depth image
	VkFormat _depthFormat;
	VkShaderModule _vert_shader;
	VkShaderModule _frag_shader;
	VkPipelineLayout _pipeline_layout;
	VkPipelineLayout _pipeline_layout_non_blend;
	VkPipeline _graphics_pipeline;
	VkPipeline _graphics_pipeline_non_blend;

	VkDescriptorPool _descriptorPool;

	VkDescriptorSetLayout _globalSetLayout;
	VkDescriptorSetLayout _objectSetLayout;
	VkDescriptorSetLayout _singleTextureSetLayout;
	UploadContext _uploadContext;
	
	void init(SDL_Window* window, int width, int height);
	void cleanup();

	void begin();
	void end();

	void imgui_init();
	void imgui_begin();
	void imgui_end();

	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	size_t pad_uniform_buffer_size(size_t originalSize);

	FrameData& get_current_frame();
	FrameData& get_last_frame();

	static VulkanEngine* GetInstance();
	static void Release();

	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
	void queue_submit(SubmitQueueInfo info);
	void flush_queue();

	std::vector<SubmitQueueInfo> _queueInfos;

	int _vertexBufferSize;
	int _indexBufferSize;
	VkBuffer _vertexBuffer;
	VkBuffer _indexBuffer;
	VkDeviceMemory _vertexBufferMemory;
	VkDeviceMemory _indexBufferMemory;

	void delete_on_present(std::function<void()>&& function);
	void re_init_swapchains(int width, int height);

	DeletionQueue _perFrameDeletionQueue;
private:
	VulkanEngine() = default;
	~VulkanEngine();

	static VulkanEngine* m_instance;

	void init_vulkan();

	bool init_swapchain();

	void init_default_renderpass();

	bool init_framebuffers();

	void init_commands();

	void init_sync_structures();

	void init_descriptors();

	void init_shaders();

	void init_pipeline();
};