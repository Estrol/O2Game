#include "EnvironmentSetup.hpp"
#include <unordered_map>

namespace {
	std::unordered_map<std::string, std::string> m_stores;
	std::unordered_map<std::string, void*> m_storesPtr;
	std::unordered_map<std::string, std::filesystem::path> m_paths;
}

void EnvironmentSetup::Set(std::string key, std::string value) {
	m_stores[key] = value;
}

std::string EnvironmentSetup::Get(std::string key) {
	return m_stores[key];
}

void EnvironmentSetup::SetObj(std::string key, void* ptr) {
	m_storesPtr[key] = ptr;
}

void* EnvironmentSetup::GetObj(std::string key) {
	return m_storesPtr[key];
}

void EnvironmentSetup::SetPath(std::string key, std::filesystem::path path) {
	m_paths[key] = path;
}

std::filesystem::path EnvironmentSetup::GetPath(std::string key) {
	return m_paths[key];
}

