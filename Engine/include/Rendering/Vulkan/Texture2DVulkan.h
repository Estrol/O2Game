#pragma once
#ifdef __GNUC__
//#include <experimental/filesystem>
#include <filesystem>
#else
#include <filesystem>
#endif

#include <stdint.h>
#include <fstream>
#include <vulkan/vulkan.h>

// Forward declaration, intended only for HANDLE only!
// See: Texture2DVulkan_Internal.h
struct Texture2D_Vulkan;

namespace vkTexture {
	uint32_t FindMemoryType(uint32_t type_filter, uint32_t properties);

	Texture2D_Vulkan* TexLoadImage(std::filesystem::path imagePath);
	Texture2D_Vulkan* TexLoadImage(void* buffer, size_t size);
	Texture2D_Vulkan* GetDummyImage();
	VkDescriptorSet GetVkDescriptorSet(Texture2D_Vulkan* image);

	void QueryTexture(Texture2D_Vulkan* handle, int& outWidth, int& outHeight);
	void ReleaseTexture(Texture2D_Vulkan* handle);

	void Cleanup();
}