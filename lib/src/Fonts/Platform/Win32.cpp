/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#if _WIN32
#include "Win32.h"
using namespace Fonts::Platform;

#ifdef _WIN32
#include <Windows.h>
#endif

FontFileInfo Win32::FindFont(std::string file)
{
#ifndef _WIN32
    return { "", true };
#else
    std::filesystem::path windows_fonts = "C:\\Windows\\Fonts\\";
    auto                  iterator = std::filesystem::recursive_directory_iterator(windows_fonts);

    for (const auto &entry : iterator) {
        if (entry.is_regular_file() && entry.path().filename() == file) {
            return { entry.path(), false };
        }
    }

    return { "", true };
#endif
}

#endif