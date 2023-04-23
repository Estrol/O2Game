#include "Configuration.hpp"
#include <filesystem>
#include "../Data/Util/mINI.h"

namespace {
    bool IsLoaded = false;
	mINI::INIStructure Config;

	std::string CurrentSkin = "";
	mINI::INIStructure SkinConfig;

	std::string defaultConfig = "[Game]\n"
		"Window = 1280x720\n"
		"Vulkan = 0\n"
		"Skin = Default\n"
		"[KeyMapping]\n"
		"Lane1 = S\n"
		"Lane2 = D\n"
		"Lane3 = F\n"
		"Lane4 = Space\n"
		"Lane5 = J\n"
		"Lane6 = K\n"
		"Lane7 = L\n"
		"[Debug]\n"
		"Autoplay = 0\n"
		"Rate = 1.0\n";

}

void LoadConfiguration() {
	if (IsLoaded) return;

	std::filesystem::path path = std::filesystem::current_path();
	path.append("Game.ini");

	if (!std::filesystem::exists(path)) {
		std::fstream fs(path, std::ios::out);
		fs << defaultConfig;
		fs.close();
	}

	mINI::INIFile file(path.string());
	file.read(Config);

	IsLoaded = true;
}

std::string Configuration::Load(std::string key, std::string prop) {
	if (!IsLoaded) LoadConfiguration();

	return Config[key][prop];
}

void Configuration::Set(std::string key, std::string prop, std::string value) {
	if (!IsLoaded) LoadConfiguration();

	Config[key][prop] = value;

	std::filesystem::path path = std::filesystem::current_path();
	path.append("Game.ini");
	
	mINI::INIFile file(path.string());
	file.write(Config, true);
}

std::string Configuration::Skin_LoadValue(std::string name, std::string key, std::string prop) {
	if (CurrentSkin != name) {
		CurrentSkin = name;

		std::filesystem::path path = std::filesystem::current_path() / "Skins" / name / "GameSkin.ini";

		mINI::INIFile f(path);
		SkinConfig.clear();
		f.read(SkinConfig);
	}

	return SkinConfig[key][prop];
}

bool Configuration::Skin_Exist(std::string name) {
	std::filesystem::path path = std::filesystem::current_path() / "Skins" / name;
	return std::filesystem::exists(path);
}
