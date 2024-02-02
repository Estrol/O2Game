/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __PLATFORMBASE_H_
#define __PLATFORMBASE_H_
#include <filesystem>
#include <string>

namespace Fonts::Platform {
    struct FontFileInfo
    {
        std::filesystem::path path;
        bool                  error;
    };
} // namespace Fonts::Platform

#endif