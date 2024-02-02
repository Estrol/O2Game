/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "Env.h"
#include <unordered_map>

namespace {
    std::unordered_map<std::string, std::string> env;
    std::unordered_map<std::string, int>         env_int;
    std::unordered_map<std::string, float>       env_float;
    std::unordered_map<std::string, bool>        env_bool;
    std::unordered_map<std::string, std::filesystem::path> paths;

    std::unordered_map<std::string, void *> pointers;
} // namespace

std::string Env::GetString(const std::string &key)
{
    return env[key];
}

int Env::GetInt(const std::string &key)
{
    if (env_int.find(key) == env_int.end())
        return 0;

    return env_int[key];
}

float Env::GetFloat(const std::string &key)
{
    if (env_float.find(key) == env_float.end())
        return 0.0f;

    return env_float[key];
}

bool Env::GetBool(const std::string &key)
{
    if (env_bool.find(key) == env_bool.end())
        return false;

    return env_bool[key];
}

std::filesystem::path Env::GetPath(const std::string& key)
{
    if (paths.find(key) == paths.end())
		return std::filesystem::path();

	return paths[key];
}

void *Env::GetPointer(const std::string &key)
{
    if (pointers.find(key) == pointers.end())
        return nullptr;

    return pointers[key];
}

void Env::SetString(const std::string &key, const std::string &value)
{
    env[key] = value;
}

void Env::SetInt(const std::string &key, int value)
{
    env_int[key] = value;
}

void Env::SetFloat(const std::string &key, float value)
{
    env_float[key] = value;
}

void Env::SetBool(const std::string &key, bool value)
{
    env_bool[key] = value;
}

void Env::SetPath(const std::string& key, const std::filesystem::path& value)
{
	paths[key] = value;
}

void Env::SetPointer(const std::string &key, void *value)
{
    pointers[key] = value;
}