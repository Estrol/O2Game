#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <string.h>

struct Texture2D_Vulkan {
#if _DEBUG && MEM_LEAK_DEBUG
	char			Memory[20] = "Texture2D";
#endif

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