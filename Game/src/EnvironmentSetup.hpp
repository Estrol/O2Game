#pragma once
#include <string>

// if GCC
#include <filesystem>

namespace EnvironmentSetup {
	void Set(std::string key, std::string value);
	std::string Get(std::string key);

	void SetObj(std::string key, void* ptr);
	void* GetObj(std::string key);

	void SetInt(std::string key, int value);
	int GetInt(std::string key);

	void SetPath(std::string key, std::filesystem::path path);
	std::filesystem::path GetPath(std::string key);
}