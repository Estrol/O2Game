#pragma once
#include <vulkan/vulkan.h>
#include <memory>

struct Texture2D_Vulkan {
	int 			Id;

	VkDescriptorSet DS;
	int             Width;
	int             Height;
	int             Channels;

	VkImageView     ImageView;
	VkImage         Image;
	VkDeviceMemory  ImageMemory;
	VkSampler       Sampler;

	VkBuffer		UploadBuffer;
	VkDeviceMemory	UploadBufferMemory;

	Texture2D_Vulkan() { memset(this, 0, sizeof(*this)); }
};