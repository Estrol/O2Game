#pragma once
#include <vulkan/vulkan.h>
#include <memory>

struct Texture2D_Vulkan {
	VkDescriptorSet DS;
	int             Width;
	int             Height;
	int             Channels;

	VkImageView     ImageView;
	VkImage         Image;
	VkDeviceMemory  ImageMemory;
	VkSampler       Sampler;

	VkBuffer		VertexBuffer;
	VkDeviceMemory	VertexBufferMemory;
	size_t			VertexBufferSize;

	VkBuffer		IndexBuffer;
	VkDeviceMemory	IndexBufferMemory;
	size_t			IndexBufferSize;

	VkBuffer		UploadBuffer;
	VkDeviceMemory	UploadBufferMemory;

	Texture2D_Vulkan() { memset(this, 0, sizeof(*this)); }
};