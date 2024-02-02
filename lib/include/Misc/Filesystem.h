/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_

#ifdef WINAPI
#undef ReadFile
#endif

#include <filesystem>
#include <vector>

namespace Misc {
    namespace Filesystem {
        std::vector<uint8_t>  ReadFile(std::filesystem::path path);
        std::vector<uint16_t> ReadFile16(std::filesystem::path path);
        std::string           ReadFileString(std::filesystem::path path);

        void WriteFile(std::filesystem::path path, const std::vector<uint8_t> &data);
        void WriteFile16(std::filesystem::path path, const std::vector<uint16_t> &data);
        void WriteFileString(std::filesystem::path path, const std::string &data);
    } // namespace Filesystem
} // namespace Misc

#endif