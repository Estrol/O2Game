#include "Configuration.h"
#include "Misc/mINI.h"
#include <Logs.h>
#include <iostream>

namespace {
    bool               IsLoaded = false;
    mINI::INIStructure Config;

    std::string        CurrentSkin = "";
    mINI::INIStructure SkinConfig;

    std::string           defaultConfig;
    std::filesystem::path FontPath;
} // namespace

void Configuration::SetDefaultConfiguration(std::string conf)
{
    defaultConfig = conf;
}

void Configuration::ResetConfiguration()
{
    std::filesystem::path path = std::filesystem::current_path() / "Game.ini";

    if (std::filesystem::exists(path)) {
        Logs::Puts("[Configuration] Deleting configuration at path: %s", path.string().c_str());
        std::filesystem::remove(path);
    }

    {
        std::fstream fs(path, std::ios::out);
        fs << defaultConfig;
        fs.close();
    }

    mINI::INIFile file(path);
    file.read(Config);

    IsLoaded = true;
}

void LoadConfiguration()
{
    if (IsLoaded)
        return;

    std::filesystem::path path = std::filesystem::current_path() / "Game.ini";

    if (!std::filesystem::exists(path)) {
        Logs::Puts("[Configuration] Creating default configuration at path: %s", path.string().c_str());

        std::fstream fs(path, std::ios::out);
        fs << defaultConfig;
        fs.close();
    }

    mINI::INIFile file(path);
    file.read(Config);

    IsLoaded = true;
}

std::string Configuration::Load(std::string key, std::string prop)
{
    if (!IsLoaded)
        LoadConfiguration();

    return Config[key][prop];
}

void Configuration::Set(std::string key, std::string prop, std::string value)
{
    if (!IsLoaded)
        LoadConfiguration();

    Config[key][prop] = value;

    std::filesystem::path path = std::filesystem::current_path() / "Game.ini";

    mINI::INIFile file(path);
    if (!file.write(Config, true)) {
        Logs::Puts("[Configuration] Failed to write configuration to file");
    }
}

void Configuration::Font_SetPath(std::filesystem::path path)
{
    FontPath = path;
}

std::filesystem::path Configuration::Font_GetPath()
{
    return FontPath;
}
