#include "SkinManager.hpp"
#include <iostream>
#include <filesystem>
#include "../Data/Util/mINI.h"

namespace {
	const std::string kSkinFolder = "Skins";
	const std::string kSkinConfig = "GameSkin.ini";

	const std::vector<std::string> Keys = {
		"#COMMON",
	};

	std::unordered_map<std::string, SkinManager::Skin*> vSkins;
}

using namespace SkinManager;

//Skin* SkinManager::CurrentSkin = nullptr;

Skin::Skin(std::string _path) {
	m_currentPath = _path;

	mINI::INIFile file((m_currentPath / kSkinConfig).string());
	mINI::INIStructure ini;
	file.read(ini);

	
}

int Skin::GetWidth() {
	return m_width;
}

int Skin::GetHeight() {
	return m_height;
}

std::vector<uint8_t>* Skin::GetComponent(std::string componentName, std::string name, ComponentType type) {
	

	return nullptr;
}

bool SelectSkin(std::string name) {
	if (vSkins.find(name) == vSkins.end()) {
		return false;
	}

	CurrentSkin = vSkins[name];
	return true;
}