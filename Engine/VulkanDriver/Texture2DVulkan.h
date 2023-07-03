#pragma once
#include <filesystem>

// Forward declaration, intended only for HANDLE only!
// See: Texture2DVulkan_Internal.h
struct Texture2D_Vulkan;

namespace vkTexture {
	uint32_t FindMemoryType(uint32_t type_filter, uint32_t properties);

	Texture2D_Vulkan* TexLoadImage(std::filesystem::path imagePath);
	Texture2D_Vulkan* TexLoadImage(void* buffer, size_t size);

	void QueryTexture(Texture2D_Vulkan* handle, int& outWidth, int& outHeight);
	void ReleaseTexture(Texture2D_Vulkan* handle);
}