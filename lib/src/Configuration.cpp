/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Configuration.h>
#include <Misc/ini.h>
#include <filesystem>
#include <fstream>

using namespace Misc::mINI;

namespace {
    std::string s_ConfigurationFile = "Game.ini";
    std::string s_DefaultContents = "";
} // namespace

void Configuration::SetDefaultConfiguration(std::string conf)
{
    s_DefaultContents = conf;
}

void Configuration::ResetConfiguration()
{
    auto path = std::filesystem::current_path() / s_ConfigurationFile;
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }

    std::ofstream fs(path);
    fs << s_DefaultContents;
    fs.close();
}

void Configuration::Set(std::string section, std::string key, std::string value)
{
    auto path = std::filesystem::current_path() / s_ConfigurationFile;
    if (!std::filesystem::exists(path)) {
        ResetConfiguration();
    }

    INIFile      file(path.string());
    INIStructure ini;
    file.read(ini);

    ini[section][key] = value;

    file.write(ini);
}

void Configuration::SetInt(std::string section, std::string key, int value)
{
    Set(section, key, std::to_string(value));
}

void Configuration::SetFloat(std::string section, std::string key, float value)
{
    Set(section, key, std::to_string(value));
}

void Configuration::SetBool(std::string section, std::string key, bool value)
{
    Set(section, key, value ? "true" : "false");
}

std::string Configuration::Get(std::string section, std::string key)
{
    auto path = std::filesystem::current_path() / s_ConfigurationFile;
    if (!std::filesystem::exists(path)) {
        ResetConfiguration();
    }

    INIFile      file(path.string());
    INIStructure ini;
    file.read(ini);

    return ini[section][key];
}

int Configuration::GetInt(std::string section, std::string key)
{
    return std::stoi(Get(section, key));
}

float Configuration::GetFloat(std::string section, std::string key)
{
    return std::stof(Get(section, key));
}

bool Configuration::GetBool(std::string section, std::string key)
{
    return Get(section, key) == "true" || Get(section, key) == "1";
}