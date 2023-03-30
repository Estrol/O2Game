#include "EnvironmentSetup.hpp"
#include <unordered_map>

namespace {
	std::unordered_map<std::string, std::string> m_stores;
}

void EnvironmentSetup::Set(std::string key, std::string value) {
	m_stores[key] = value;
}

std::string EnvironmentSetup::Get(std::string key) {
	return m_stores[key];
}
