#pragma once
#include <string>

namespace EnvironmentSetup {
	void Set(std::string key, std::string value);
	std::string Get(std::string key);
}