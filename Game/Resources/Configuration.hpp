#pragma once
#include <string>

namespace Configuration {
	std::string Load(std::string key, std::string prop);
	void Set(std::string key, std::string prop, std::string value);

	std::string Skin_LoadValue(std::string name, std::string key, std::string prop);
	bool Skin_Exist(std::string name);
}