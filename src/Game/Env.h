/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include <filesystem>
#include <string>

namespace Env {
    std::string                GetString(const std::string &key);
    int                        GetInt(const std::string &key);
    float                      GetFloat(const std::string &key);
    bool                       GetBool(const std::string &key);
    std::filesystem::path      GetPath(const std::string &key);
    void                      *GetPointer(const std::string &key);
    std::vector<unsigned char> GetBuffer(const std::string &key);

    void SetString(const std::string &key, const std::string &value);
    void SetInt(const std::string &key, int value);
    void SetFloat(const std::string &key, float value);
    void SetBool(const std::string &key, bool value);
    void SetPath(const std::string &key, const std::filesystem::path &value);
    void SetPointer(const std::string &key, void *value);
    void SetBuffer(const std::string &key, const std::vector<unsigned char> &value);
} // namespace Env