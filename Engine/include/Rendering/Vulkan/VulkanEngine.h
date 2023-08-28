#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <deque>
#include <unordered_map>
#include "vk_mem_alloc/vk_mem_alloc.h"
#include "VkBootstrap/VkBootstrap.h"
#include "Rendering/WindowsTypes.h"

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
	const char SIGNATURE[25] = "AllocatedImage";

	VkImage _image;
	VmaAllocation _allocation;
};

struct AllocatedBuffer {
	const char SIGNATURE[25] = "AllocatedBuffer";
	VkBuffer _buffer;
	VmaAllocation _allocation;
};

struct FrameData {
	const char SIGNATURE[25] = "FrameData";
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
	const char SIGNATURE[25] = "UploadContext";
	VkFence _uploadFence;
	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;
};

struct ImDrawVert;

struct SubmitQueueInfo {
	const char SIGNATURE[25] = "SubmitQueueInfo";
	std::vector<ImDrawVert> vertices;
	std::vector<uint16_t> indices;

	VkRect2D scissor;
	bool AlphaBlend;
	VkDescriptorSet descriptor;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine {
	const char SIGNATURE[25] = "VulkanEngine";

public:
	bool _isInitialized{ false };
	bool _swapChainOutdated{ false };
	int _frameNumber{ 0 };
	uint32_t _swapchainNumber{ 0 };

	VkExtent2D _windowExtent{ 1700 , 900 };

	struct SDL_Window* _window{ nullptr };

	VkInstance _instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT _debug_messenger = VK_NULL_HANDLE;
	VkPhysicalDevice _chosenGPU = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;

	VkPhysicalDeviceProperties _gpuProperties;

	FrameData _frames[FRAME_OVERLAP];

	VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamily;

	VkRenderPass _renderPass = VK_NULL_HANDLE;

	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	vkb::Swapchain _swapchainData;
	VkFormat _swachainImageFormat;

	std::vector<VkFramebuffer> _framebuffers;
	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;

	DeletionQueue _mainDeletionQueue;
	DeletionQueue _swapChainQueue;
	VmaAllocator _allocator = VK_NULL_HANDLE;

	VkImageView _depthImageView = VK_NULL_HANDLE;
	AllocatedImage _depthImage;

	//the format for the depth image
	VkFormat _depthFormat;
	VkShaderModule _vert_shader = VK_NULL_HANDLE;
	VkShaderModule _frag_shader = VK_NULL_HANDLE;
	VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;
	VkPipelineLayout _pipeline_layout_non_blend = VK_NULL_HANDLE;
	VkPipeline _graphics_pipeline = VK_NULL_HANDLE;
	VkPipeline _graphics_pipeline_non_blend = VK_NULL_HANDLE;

	VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;

	VkDescriptorSetLayout _globalSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _objectSetLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout _singleTextureSetLayout = VK_NULL_HANDLE;
	UploadContext _uploadContext;
	
	void init(SDL_Window* window, int width, int height);
	void cleanup();

	void begin();
	void end();

	void imgui_init();
	void imgui_begin();
	void imgui_end();

	void set_vsync(bool v);

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

	int _vertexBufferSize = 0;
	int _indexBufferSize = 0;
	VkBuffer _vertexBuffer = VK_NULL_HANDLE;
	VkBuffer _indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory _vertexBufferMemory = VK_NULL_HANDLE;
	VkDeviceMemory _indexBufferMemory = VK_NULL_HANDLE;
	VkAllocationCallbacks* _allocCallback = VK_NULL_HANDLE;
	
	void re_init_swapchains(int width, int height);

	DeletionQueue _perFrameDeletionQueue;
private:
	VulkanEngine() = default;
	~VulkanEngine();

	bool m_vsync = false;
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