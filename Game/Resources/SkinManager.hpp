#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>

/* Enum types */
enum class ComponentType {
	IMAGE,
	BND
};

// Skin handle, cast it to specific struct based on ComponentType
typedef void* CHANDLE;

namespace SkinManager {
	class Skin {
	public:
		Skin() = default;
		Skin(std::string path);

		int GetWidth();
		int GetHeight();

		std::vector<uint8_t>* GetComponent(std::string componentName, std::string name, ComponentType type = ComponentType::IMAGE);

	private:
		bool m_isOPI;

		int m_width, m_height;
		
		std::filesystem::path m_currentPath;
		std::unordered_map<std::string, std::vector<uint8_t>> m_cache;
	};

	bool SelectSkin(std::string name);
	Skin* CurrentSkin;
}