#pragma once
#include <string>
#include <filesystem>

namespace Configuration {
	void SetDefaultConfiguration(std::string conf);
	void ResetConfiguration();

	std::string Load(std::string key, std::string prop);
	void Set(std::string key, std::string prop, std::string value);

	std::filesystem::path Skin_GetPath(std::string name);
	std::string Skin_LoadValue(std::string name, std::string key, std::string prop);
	bool Skin_Exist(std::string name);
}